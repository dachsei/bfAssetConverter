#pragma once
#include "Skeleton.h"
#include "Utils.h"

class SkinnedMesh
{
public:
	SkinnedMesh(std::istream& stream, const Skeleton& skeleton);
	~SkinnedMesh() = default;

	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const;

private:
	struct MeshBone {
		uint32_t id;
		glm::mat4 matrix;
	};
	struct Rig {
		std::vector<MeshBone> bones;
	};
	struct Material {
		enum Alphamode : uint32_t { opaque = 0, blend = 1, alphatest = 2 };
		Alphamode alphamode;
		std::string fxFile;
		std::string technique;

		std::vector<std::string> map;

		uint32_t vertexOffset;
		uint32_t indexOffset;
		uint32_t vertexCount;
		uint32_t indexCount;
	};
	struct Lod {
		glm::vec3 min;
		glm::vec3 max;
		glm::vec3 pivot;
		std::vector<Rig> rigs;
		std::vector<Material> materials;
	};
	struct Geometry {
		std::vector<Lod> lods;
	};
	struct VertexAttrib {
		uint16_t flag;
		uint16_t offset;
		enum Vartype : uint16_t { float1 = 0, float2 = 1, float3 = 2, d3dcolor = 4 };
		Vartype vartype;
		enum Usage : uint16_t { position = 0, blendWeight = 1, blendIndices = 2, normal = 3, uv1 = 5, tangent = 6 };
		Usage usage;
	};

	void readRigs(std::istream& stream, Lod& lod) const;
	void readMaterials(std::istream& stream, Lod& lod) const;

	char* writeGeometry(rapidxml::xml_document<>& doc, rapidxml::xml_node<> *libraryGeometries, const std::string& objectName, const Material& material) const;
	char* writeSkinController(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* libraryControllers, const std::string& objectName, const Material& material, const Rig& rig, const char* meshId) const;
	void writeSceneObject(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* visualScene, const std::string& objectName, const char* skinId) const;
	char* writeValueNtimes(rapidxml::xml_document<>& doc, size_t count, char* value) const;
	std::pair<char*, size_t> computeIndices(rapidxml::xml_document<>& doc, const Material& material, size_t inputCount) const;
	std::pair<char*, size_t> writeBoneNames(rapidxml::xml_document<>& doc, const Rig& rig) const;
	std::pair<char*, size_t> writeBonePoses(rapidxml::xml_document<>& doc, const Rig& rig) const;
	std::pair<char*, size_t> writeVertexData(rapidxml::xml_document<>& doc, const Material& material, VertexAttrib::Usage usage) const;
	size_t computeVertexWeights(const Material& material, std::vector<float>& weightData, std::vector<size_t>& indexData) const;

	const Skeleton& skeleton;

	uint32_t version;
	std::vector<Geometry> geometrys;
	std::vector<VertexAttrib> vertexAttribs;
	uint32_t vertexformat;
	uint32_t vertexstride;
	std::vector<float> vertices;
	std::vector<uint16_t> indices;
};