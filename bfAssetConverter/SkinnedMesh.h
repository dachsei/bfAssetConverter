#pragma once
#include "Skeleton.h"
#include "Mesh.h"

class SkinnedMesh : public Mesh
{
public:
	SkinnedMesh(std::istream& stream, const Skeleton& skeleton);
	~SkinnedMesh() = default;

	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const;

protected:
	void readRigs(std::istream& stream, Lod& lod) const;

	char* writeGeometry(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* libraryGeometries, const std::string& objectName, const Material& material) const;
	char* writeSkinController(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* libraryControllers, const std::string& objectName, const Material& material, const Rig& rig, const char* meshId) const;
	void writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* skinId) const;
	char* writeValueNtimes(rapidxml::xml_document<>& doc, size_t count, char* value) const;
	std::pair<char*, size_t> computeIndices(rapidxml::xml_document<>& doc, const Material& material, size_t inputCount) const;
	std::pair<char*, size_t> writeBoneNames(rapidxml::xml_document<>& doc, const Rig& rig) const;
	std::pair<char*, size_t> writeBonePoses(rapidxml::xml_document<>& doc, const Rig& rig) const;
	std::pair<char*, size_t> writeVertexData(rapidxml::xml_document<>& doc, const Material& material, VertexAttrib::Usage usage) const;
	size_t computeVertexWeights(const Material& material, std::vector<float>& weightData, std::vector<size_t>& indexData) const;

	const Skeleton& skeleton;
};