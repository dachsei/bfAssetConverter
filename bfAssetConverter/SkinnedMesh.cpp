#include "SkinnedMesh.h"
#include <sstream>
#include <map>

using namespace Utils;
using namespace rapidxml;

SkinnedMesh::SkinnedMesh(std::istream& stream, const Skeleton& skeleton)
	:skeleton(skeleton)
{
	stream.ignore(1 * 4);	//unused
	readBinary(stream, &version);
	stream.ignore(3 * 4);

	//Geometry table
	stream.ignore(1);
	uint32_t geomCount;
	readBinary(stream, &geomCount);
	geometrys.resize(geomCount);
	for (Geometry& it : geometrys) {
		uint32_t lodCount;
		readBinary(stream, &lodCount);
		it.lods.resize(lodCount);
	}

	//Vertex attribute table
	uint32_t vertexAttributeCount;
	readBinary(stream, &vertexAttributeCount);
	vertexAttribs.resize(vertexAttributeCount);
	readBinaryArray(stream, vertexAttribs.data(), vertexAttributeCount);

	//Vertices
	readBinary(stream, &vertexformat);
	readBinary(stream, &vertexstride);
	uint32_t vertexCount;
	readBinary(stream, &vertexCount);
	vertices.resize((vertexstride / vertexformat)*vertexCount);
	readBinaryArray(stream, vertices.data(), vertices.size());

	//Indices
	uint32_t indexCount;
	readBinary(stream, &indexCount);
	indices.resize(indexCount);
	readBinaryArray(stream, indices.data(), indices.size());

	//Rigs
	for (Geometry& geom : geometrys) {
		for (Lod& lod : geom.lods) {
			readRigs(stream, lod);
		}
	}

	//Triangles
	for (Geometry& geom : geometrys) {
		for (Lod& lod : geom.lods) {
			readMaterials(stream, lod);
		}
	}
}

void SkinnedMesh::writeToCollada(xml_document<>& doc, xml_node<>* root) const
{
	xml_node<>* libraryGeometries = root->first_node("library_geometries");
	xml_node<>* libraryControllers = root->first_node("library_controllers");
	xml_node<>* visualScene = root->first_node("library_visual_scenes")->first_node("visual_scene");

	size_t objectId = 0;
	for (const Geometry& geom : geometrys) {
		for (const Lod& lod : geom.lods) {
			for (size_t iMaterial = 0; iMaterial < lod.materials.size(); ++iMaterial) {
				const Material& material = lod.materials[iMaterial];
				std::string objectName = "Object_" + std::to_string(objectId);
				char* meshId = writeGeometry(doc, libraryGeometries, objectName, material);
				char* skinId = writeSkinController(doc, libraryControllers, objectName, material, lod.rigs[iMaterial], meshId);
				writeSceneObject(doc, visualScene, objectName, skinId);
				++objectId;
			}
		}
	}
}

void SkinnedMesh::readRigs(std::istream& stream, Lod& lod) const
{
	readBinary(stream, &lod.min);
	readBinary(stream, &lod.max);
	if (version <= 6)
		readBinary(stream, &lod.pivot);

	uint32_t rigCount;
	readBinary(stream, &rigCount);
	lod.rigs.resize(rigCount);
	for (Rig& rig : lod.rigs) {
		uint32_t boneCount;
		readBinary(stream, &boneCount);
		rig.bones.resize(boneCount);
		for (MeshBone& bone : rig.bones) {
			readBinary(stream, &bone.id);
			readBinary(stream, &bone.matrix);
		}
	}
}

void SkinnedMesh::readMaterials(std::istream& stream, Lod& lod) const
{
	uint32_t materialCount;
	readBinary(stream, &materialCount);
	lod.materials.resize(materialCount);
	for (Material& material : lod.materials) {
		material.fxFile = readStringFormat2(stream);
		material.technique = readStringFormat2(stream);

		uint32_t mappingCount;
		readBinary(stream, &mappingCount);
		material.map.resize(mappingCount);
		for (std::string& name : material.map) {
			name = readStringFormat2(stream);
		}

		readBinary(stream, &material.vertexOffset);
		readBinary(stream, &material.indexOffset);
		readBinary(stream, &material.indexCount);
		readBinary(stream, &material.vertexCount);

		stream.ignore(2 * 4);
	}
}

char *SkinnedMesh::writeGeometry(xml_document<>& doc, xml_node<>* libraryGeometries, const std::string& objectName, const Material& material) const
{
	xml_node<>* geometry = doc.allocate_node(node_element, "geometry");
	char* meshId = setId(doc, geometry, objectName + "-mesh");
	{
		xml_node<>* mesh = doc.allocate_node(node_element, "mesh");
		{
			std::pair<char*, size_t> positionData = writeVertexData(doc, material, VertexAttrib::position);
			char* positionsId = writeSourceNode(doc, mesh, objectName + "-mesh-positions", positionData.first, positionData.second, Format::xyz);
			std::pair<char*, size_t> normalData = writeVertexData(doc, material, VertexAttrib::normal);
			char* normalsId = writeSourceNode(doc, mesh, objectName + "-mesh-normals", normalData.first, normalData.second, Format::xyz);
			std::pair<char*, size_t> texData = writeVertexData(doc, material, VertexAttrib::uv1);
			char* texId = writeSourceNode(doc, mesh, objectName + "-mesh-map", texData.first, texData.second, Format::st);

			xml_node<>* vertices = doc.allocate_node(node_element, "vertices");
			char* verticesId = setId(doc, vertices, objectName + "-mesh-vertices");
			{
				xml_node<>* input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "POSITION"));
				input->append_attribute(doc.allocate_attribute("source", positionsId));
				vertices->append_node(input);
			}
			mesh->append_node(vertices);

			xml_node<>* polylist = doc.allocate_node(node_element, "polylist");
			{
				size_t polyCount;
				char* indexData;
				std::tie(indexData, polyCount) = computeIndices(doc, material, 3);
				polylist->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(polyCount).c_str())));

				xml_node<> *input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "VERTEX"));
				input->append_attribute(doc.allocate_attribute("source", verticesId));
				input->append_attribute(doc.allocate_attribute("offset", "0"));
				polylist->append_node(input);
				input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "NORMAL"));
				input->append_attribute(doc.allocate_attribute("source", normalsId));
				input->append_attribute(doc.allocate_attribute("offset", "1"));
				polylist->append_node(input);
				input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "TEXCOORD"));
				input->append_attribute(doc.allocate_attribute("source", texId));
				input->append_attribute(doc.allocate_attribute("offset", "2"));
				polylist->append_node(input);

				polylist->append_node(doc.allocate_node(node_element, "vcount", writeValueNtimes(doc, polyCount, "3")));
				polylist->append_node(doc.allocate_node(node_element, "p", indexData));
			}
			mesh->append_node(polylist);
		}
		geometry->append_node(mesh);
	}
	libraryGeometries->append_node(geometry);
	return meshId;
}

char* SkinnedMesh::writeSkinController(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* libraryControllers, const std::string& objectName, const Material& material, const Rig& rig, const char* meshId) const
{
	xml_node<>* controller = doc.allocate_node(node_element, "controller");
	char* skinId = setId(doc, controller, objectName + "-skin");
	{
		xml_node<>* skin = doc.allocate_node(node_element, "skin");
		{
			skin->append_attribute(doc.allocate_attribute("source", meshId));

			std::pair<char*, size_t> jointData = writeBoneNames(doc, rig);
			char* jointsId = writeSourceNode(doc, skin, objectName + "-skin-joints", jointData.first, jointData.second, Format::joint);
			std::pair<char*, size_t> poseData = writeBonePoses(doc, rig);
			char* posesId = writeSourceNode(doc, skin, objectName + "-skin-poses", poseData.first, poseData.second, Format::transform);
			std::vector<float> weightData;
			std::vector<size_t> indexData;
			size_t vertexCount = computeVertexWeights(material, weightData, indexData);
			char* weightsId = writeSourceNode(doc, skin, objectName + "-skin-weights", floatsToString(doc, weightData), weightData.size(), Format::weight);

			xml_node<>* joints = doc.allocate_node(node_element, "joints");
			{
				xml_node<>* input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "JOINT"));
				input->append_attribute(doc.allocate_attribute("source", jointsId));
				joints->append_node(input);
				input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "INV_BIND_MATRIX"));
				input->append_attribute(doc.allocate_attribute("source", posesId));
				joints->append_node(input);
			}
			skin->append_node(joints);

			xml_node<>* vertexWeights = doc.allocate_node(node_element, "vertex_weights");
			{
				vertexWeights->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(vertexCount).c_str())));
				xml_node<>* input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "JOINT"));
				input->append_attribute(doc.allocate_attribute("source", jointsId));
				input->append_attribute(doc.allocate_attribute("offset", "0"));
				vertexWeights->append_node(input);
				input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "WEIGHT"));
				input->append_attribute(doc.allocate_attribute("source", weightsId));
				input->append_attribute(doc.allocate_attribute("offset", "1"));
				vertexWeights->append_node(input);

				vertexWeights->append_node(doc.allocate_node(node_element, "vcount", writeValueNtimes(doc, vertexCount, "2")));
				vertexWeights->append_node(doc.allocate_node(node_element, "v", indicesToString(doc, indexData)));
			}
			skin->append_node(vertexWeights);
		}
		controller->append_node(skin);
	}
	libraryControllers->append_node(controller);
	return skinId;
}

void SkinnedMesh::writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* skinId) const
{
	xml_node<>* node = doc.allocate_node(node_element, "node");
	{
		char* id = setId(doc, node, objectName);
		node->append_attribute(doc.allocate_attribute("name", id + 1));
		node->append_attribute(doc.allocate_attribute("type", "NODE"));

		xml_node<>* instanceController = doc.allocate_node(node_element, "instance_controller");
		{
			instanceController->append_attribute(doc.allocate_attribute("url", skinId));
			std::string rootSkeletonId = "#" + skeleton.bones[0].name;
			instanceController->append_node(doc.allocate_node(node_element, "skeleton",
				doc.allocate_string(rootSkeletonId.c_str(), rootSkeletonId.length() + 1)));
		}
		node->append_node(instanceController);
	}
	visualScene->append_node(node);
}

char* SkinnedMesh::writeValueNtimes(rapidxml::xml_document<>& doc, size_t count, char* value) const
{
	std::stringstream ss;
	for (size_t i = 0; i < count; ++i) {
		ss << value << " ";
	}
	std::string result = ss.str();
	result.pop_back();
	return doc.allocate_string(result.c_str(), result.length() + 1);
}

std::pair<char*, size_t> SkinnedMesh::computeIndices(rapidxml::xml_document<>& doc, const Material& material, size_t inputCount) const
{
	std::stringstream ss;
	size_t polycount = 0;
	for (size_t i = 0; i < material.indexCount; ++i) {
		for (size_t input = 0; input < inputCount; ++input) {
			ss << indices[material.indexOffset + i] << " ";
		}
	}
	polycount += material.indexCount / 3;

	std::string result = ss.str();
	result.pop_back();
	return std::pair<char*, size_t>(doc.allocate_string(result.c_str(), result.length() + 1), polycount);
}

std::pair<char*, size_t> SkinnedMesh::writeBoneNames(rapidxml::xml_document<>& doc, const Rig& rig) const
{
	std::stringstream ss;
	size_t count = 0;
	for (const MeshBone& bone : rig.bones) {
		ss << skeleton.bones[bone.id].name << " ";
	}
	count += rig.bones.size();

	std::string data = ss.str();
	data.pop_back();
	return std::pair<char*, size_t>(doc.allocate_string(data.c_str(), data.length() + 1), count);
}

std::pair<char*, size_t> SkinnedMesh::writeBonePoses(rapidxml::xml_document<>& doc, const Rig& rig) const
{
	std::stringstream ss;
	size_t count = 0;
	for (const MeshBone& bone : rig.bones) {
		writeMatrixToStream(ss, bone.matrix);
	}
	count += rig.bones.size();

	std::string data = ss.str();
	data.pop_back();

	return std::pair<char*, size_t>(doc.allocate_string(data.c_str(), data.length() + 1), count);
}

std::pair<char*, size_t> SkinnedMesh::writeVertexData(rapidxml::xml_document<>& doc, const Material& material, VertexAttrib::Usage usage) const
{
	size_t offset = -1;
	size_t elementCount;
	for (const VertexAttrib& attrib : vertexAttribs) {
		if (attrib.usage == usage) {
			offset = attrib.offset / vertexformat;
			switch (attrib.vartype) {
			case VertexAttrib::float1: elementCount = 1; break;
			case VertexAttrib::float2: elementCount = 2; break;
			case VertexAttrib::float3: elementCount = 3; break;
			}
		}
	}
	assert(offset != -1);

	std::stringstream ss;
	size_t count = 0;
	for (size_t i = 0; i < material.vertexCount; ++i) {
		for (size_t elem = 0; elem < elementCount; ++elem) {
			ss << vertices[(material.vertexOffset + i)*vertexstride / vertexformat + offset + elem] << " ";
		}
		++count;
	}
	std::string result = ss.str();
	result.pop_back();
	return std::pair<char*, size_t>(doc.allocate_string(result.c_str(), result.length() + 1), count);
}

size_t SkinnedMesh::computeVertexWeights(const Material& material, std::vector<float>& weightData, std::vector<size_t>& indexData) const
{
	size_t indexOffset = -1;
	size_t weightOffset = -1;
	for (const VertexAttrib& attrib : vertexAttribs) {
		if (attrib.usage == VertexAttrib::blendIndices) {
			indexOffset = attrib.offset / vertexformat;
		}
		if (attrib.usage == VertexAttrib::blendWeight) {
			weightOffset = attrib.offset / vertexformat;
		}
	}
	assert(indexOffset != -1 && weightOffset != -1);

	std::map<float, size_t> weightIndexMap;
	size_t vertexCount = 0;
	for (size_t i = 0; i < material.vertexCount; ++i) {
		size_t vertexBase = (material.vertexOffset + i)*vertexstride / vertexformat;
		std::vector<float> weights(2);
		weights[0] = vertices[vertexBase + weightOffset];
		weights[1] = 1 - weights[0];
		glm::u8vec4 poseIndices = reinterpret_cast<const glm::u8vec4&>(vertices[vertexBase + indexOffset]);

		if (poseIndices.x == poseIndices.y) {	//Don't allow same Index twice -> change the 0 influence to any other
			if (weights[0] == 1)
				poseIndices.y = !poseIndices.y;	//1 if 0, 0 else
			else
				poseIndices.x = !poseIndices.x;
		}

		for (size_t w = 0; w < weights.size(); ++w) {
			indexData.push_back(poseIndices[w]);
			auto ins = weightIndexMap.insert(std::make_pair(weights[w], weightData.size()));
			if (ins.second)
				weightData.push_back(weights[w]);
			indexData.push_back(ins.first->second);
		}
		++vertexCount;
	}

	return vertexCount;
}
