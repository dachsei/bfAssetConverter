#include "Mesh.h"
#include <sstream>
#include <tuple>

using namespace Utils;
using namespace rapidxml;

Mesh::Mesh(std::istream& stream)
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
}

void Mesh::readMaterial(std::istream& stream, Material& material) const
{
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

void Mesh::flipTextureCoords()
{
	size_t offset = -1;
	for (const VertexAttrib& attrib : vertexAttribs) {
		if (attrib.usage == VertexAttrib::uv1) {
			offset = attrib.offset / vertexformat;
			assert(attrib.vartype == VertexAttrib::float2);
		}
	}
	assert(offset != -1);

	for (size_t i = offset + 1; i < vertices.size(); i += vertexstride / vertexformat) {	//+1 for the y component
		vertices[i] = 1 - vertices[i];
	}
}

char* Mesh::writeGeometry(xml_document<>& doc, xml_node<>* libraryGeometries, const std::string& objectName, const Material& material) const
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

std::pair<char*, size_t> Mesh::writeVertexData(xml_document<>& doc, const Material& material, VertexAttrib::Usage usage) const
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

char* Mesh::writeValueNtimes(rapidxml::xml_document<>& doc, size_t count, char* value) const
{
	std::stringstream ss;
	for (size_t i = 0; i < count; ++i) {
		ss << value << " ";
	}
	std::string result = ss.str();
	result.pop_back();
	return doc.allocate_string(result.c_str(), result.length() + 1);
}

std::pair<char*, size_t> Mesh::computeIndices(rapidxml::xml_document<>& doc, const Material& material, size_t inputCount) const
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