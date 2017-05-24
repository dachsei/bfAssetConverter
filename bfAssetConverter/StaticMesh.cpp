#include "StaticMesh.h"
#include <string>

using namespace Utils;
using namespace rapidxml;

StaticMesh::StaticMesh(std::istream& stream)
	:Mesh(stream)
{
	stream.ignore(4);

	//Lod data
	for (Geometry& geom : geometrys) {
		for (Lod& lod : geom.lods) {
			readLodNodeTable(stream, lod);
		}
	}

	//Triangles
	for (Geometry& geom : geometrys) {
		for (Lod& lod : geom.lods) {
			uint32_t materialCount;
			readBinary(stream, &materialCount);
			lod.materials.resize(materialCount);
			for (Material& material : lod.materials) {
				readBinary(stream, &material.alphamode);
				readMaterial(stream, material);
			}
		}
	}

	flipTextureCoords();
	mirrorFix();
}

void StaticMesh::writeToCollada(xml_document<>& doc, xml_node<>* root, const Lod& lod) const
{
	xml_node<>* libraryGeometries = root->first_node("library_geometries");
	xml_node<>* libraryControllers = root->first_node("library_controllers");
	xml_node<>* visualScene = root->first_node("library_visual_scenes")->first_node("visual_scene");

	xml_node<>* libraryEffects = root->first_node("library_effects");
	xml_node<>* libraryMaterials = root->first_node("library_materials");

	size_t objectId = 0;
	for (size_t iMaterial = 0; iMaterial < lod.materials.size(); ++iMaterial) {
		const Material& material = lod.materials[iMaterial];
		std::string objectName = "Object_" + std::to_string(objectId);
		char* meshId = writeGeometry(doc, libraryGeometries, objectName, material);

		xml_node<>* effect = doc.allocate_node(node_element, "effect");
		char* effectId = Utils::setId(doc, effect, objectName + "-effect");
		{
			xml_node<>* profile = doc.allocate_node(node_element, "profile_COMMON");
			{
				xml_node<>* technique = doc.allocate_node(node_element, "technique");
				{
					technique->append_attribute(doc.allocate_attribute("sid", "common"));
					technique->append_node(doc.allocate_node(node_element, "phong"));
				}
				profile->append_node(technique);
			}
			effect->append_node(profile);
		}
		libraryEffects->append_node(effect);
		xml_node<>* materialNode = doc.allocate_node(node_element, "material");
		char* materialId = Utils::setId(doc, materialNode, objectName + "-material");
		{
			xml_node<>* instanceEffect = doc.allocate_node(node_element, "instance_effect");
			{
				instanceEffect->append_attribute(doc.allocate_attribute("url", effectId));
			}
			materialNode->append_node(instanceEffect);
		}
		libraryMaterials->append_node(materialNode);

		writeSceneObject(doc, visualScene, objectName, meshId, materialId);
		++objectId;
	}
}

void StaticMesh::readLodNodeTable(std::istream& stream, Lod& lod)
{
	readBinary(stream, &lod.min);
	readBinary(stream, &lod.max);
	if (version <= 6)
		readBinary(stream, &lod.pivot);

	uint32_t nodenum;
	readBinary(stream, &nodenum);
	lod.nodes.resize(nodenum);
	for (glm::mat4& node : lod.nodes) {
		readBinary(stream, &node);
	}
}

void StaticMesh::readMaterial(std::istream& stream, Material& material) const
{
	Mesh::readMaterial(stream, material);

	/*glm::vec3 min, max;
	readBinary(stream, &min);
	readBinary(stream, &max);*/
	stream.ignore(2 * sizeof(glm::vec3));
}

void StaticMesh::writeSceneObject(xml_document<>& doc, xml_node<>* visualScene, const std::string& objectName, const char* geomId, const char* materialId) const
{
	xml_node<>* node = doc.allocate_node(node_element, "node");
	{
		char* id = setId(doc, node, objectName);
		node->append_attribute(doc.allocate_attribute("name", id + 1));
		node->append_attribute(doc.allocate_attribute("type", "NODE"));

		xml_node<>* instanceGeometry = doc.allocate_node(node_element, "instance_geometry");
		{
			instanceGeometry->append_attribute(doc.allocate_attribute("url", geomId));
			instanceGeometry->append_attribute(doc.allocate_attribute("name", id + 1));

			xml_node<>* bindMaterial = doc.allocate_node(node_element, "bind_material");
			{
				xml_node<>* techniqueCommon = doc.allocate_node(node_element, "technique_common");
				{
					xml_node<>* instanceMaterial = doc.allocate_node(node_element, "instance_material");
					instanceMaterial->append_attribute(doc.allocate_attribute("symbol", materialId + 1));	//remove #
					instanceMaterial->append_attribute(doc.allocate_attribute("target", materialId));
					techniqueCommon->append_node(instanceMaterial);
				}
				bindMaterial->append_node(techniqueCommon);
			}
			instanceGeometry->append_node(bindMaterial);
		}
		node->append_node(instanceGeometry);
	}
	visualScene->append_node(node);
}
