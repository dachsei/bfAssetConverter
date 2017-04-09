#pragma once
#include "Mesh.h"

class BundledMesh : public Mesh
{
public:
	BundledMesh(std::istream& stream);
	~BundledMesh() = default;

	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const;
	void writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* geomId) const;
};
