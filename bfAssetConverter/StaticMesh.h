#pragma once
#include "Mesh.h"

class StaticMesh : public Mesh
{
public:
	StaticMesh(std::istream& stream);
	~StaticMesh() = default;

protected:
	void readLodNodeTable(std::istream& stream, Lod& lod);
	void readMaterial(std::istream& stream, Material& material) const override;

	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root, const Lod& lod) const override;
	void writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* geomId) const;
};