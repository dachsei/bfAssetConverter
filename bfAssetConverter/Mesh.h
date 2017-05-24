#pragma once
#include "Utils.h"

class Mesh
{
public:
	Mesh(std::istream& stream);
	virtual ~Mesh() = default;

	void writeFiles(const std::string& baseName) const;

protected:
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
	struct MeshBone {
		uint32_t id;
		glm::mat4 matrix;
	};
	struct Rig {
		std::vector<MeshBone> bones;
	};
	struct Lod {
		glm::vec3 min;
		glm::vec3 max;
		glm::vec3 pivot;
		std::vector<Rig> rigs;
		std::vector<glm::mat4> nodes;
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
		enum Usage : uint16_t { position = 0, blendWeight = 1, blendIndices = 2, normal = 3, uv1 = 5, tangent = 6,
			uv2 = 0x105, uv3 = 0x205, uv4 = 0x305, uv5 = 0x405 };
		Usage usage;
	};

	virtual void readMaterial(std::istream& stream, Material& material) const;
	void flipTextureCoords();
	void mirrorFix();

	virtual void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root, const Lod& lod) const = 0;
	char* writeGeometry(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* libraryGeometries, const std::string& objectName,
		const Material& material) const;
	std::pair<char*, size_t> writeVertexData(rapidxml::xml_document<>& doc, const Material& material, VertexAttrib::Usage usage) const;
	char* writeValueNtimes(rapidxml::xml_document<>& doc, size_t count, char* value) const;
	std::pair<char*, size_t> computeIndices(rapidxml::xml_document<>& doc, const Material& material, size_t inputCount) const;

	uint32_t version;
	std::vector<Geometry> geometrys;
	std::vector<VertexAttrib> vertexAttribs;
	uint32_t vertexformat;
	uint32_t vertexstride;
	std::vector<float> vertices;
	std::vector<uint16_t> indices;
};
