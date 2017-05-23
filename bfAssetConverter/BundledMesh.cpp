#include "BundledMesh.h"
#include <string>

using namespace Utils;
using namespace rapidxml;

BundledMesh::BundledMesh(std::istream& stream)
	:Mesh(stream)
{
	stream.ignore(4);

	//Lod data
	for (Geometry& geom : geometrys) {
		for (Lod& lod : geom.lods) {
			readBinary(stream, &lod.min);
			readBinary(stream, &lod.max);
			if (version <= 6)
				readBinary(stream, &lod.pivot);
			uint32_t nodenum;
			readBinary(stream, &nodenum);
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
}

void BundledMesh::writeToCollada(xml_document<>& doc, xml_node<>* root, const Lod& lod) const
{
	xml_node<>* libraryGeometries = root->first_node("library_geometries");
	xml_node<>* libraryControllers = root->first_node("library_controllers");
	xml_node<>* visualScene = root->first_node("library_visual_scenes")->first_node("visual_scene");

	size_t objectId = 0;
	for (size_t iMaterial = 0; iMaterial < lod.materials.size(); ++iMaterial) {
		const Material& material = lod.materials[iMaterial];
		std::string objectName = "Object_" + std::to_string(objectId);
		char* meshId = writeGeometry(doc, libraryGeometries, objectName, material);
		writeSceneObject(doc, visualScene, objectName, meshId);
		++objectId;
	}
}

void BundledMesh::writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* geomId) const
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
		}
		node->append_node(instanceGeometry);
	}
	visualScene->append_node(node);
}
