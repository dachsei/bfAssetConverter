#pragma once
#include "Mesh.h"

class BundledMesh : public Mesh
{
public:
	BundledMesh(std::istream& stream);
	~BundledMesh() = default;

protected:
	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root, const Lod& lod) const override;
	void writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* geomId) const;
};
