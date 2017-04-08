#pragma once
#include "Utils.h"

class BundledMesh
{
public:
	BundledMesh(std::istream& stream);
	~BundledMesh() = default;

private:
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

	void readMaterials(std::istream& stream, Lod& lod) const;
	void flipTextureCoords();

	uint32_t version;
	std::vector<Geometry> geometrys;
	std::vector<VertexAttrib> vertexAttribs;
	uint32_t vertexformat;
	uint32_t vertexstride;
	std::vector<float> vertices;
	std::vector<uint16_t> indices;
};
