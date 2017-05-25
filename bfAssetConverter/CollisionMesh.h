#pragma once
#include "Utils.h"

class CollisionMesh
{
public:
	CollisionMesh(std::istream& stream);

protected:
	struct Face {
		uint16_t v1;
		uint16_t v2;
		uint16_t v3;
		uint16_t m;
	};
	struct Lod {
		uint32_t coltype;
		std::vector<Face> faces;
		std::vector<glm::vec3> vertices;
		std::vector<uint16_t> vertexIds;
		glm::vec3 min;
		glm::vec3 max;
		glm::vec3 bmin;
		glm::vec3 bmax;
	};
	struct SubGeometry {
		std::vector<Lod> lods;
	};
	struct Geometry {
		std::vector<SubGeometry> subGeoms;
	};

	void ReadGeometry(std::istream& stream, Geometry& geom) const;
	void ReadSubGeometry(std::istream& stream, SubGeometry& geom) const;
	void ReadLod(std::istream& stream, Lod& lod) const;

	uint32_t version;
	std::vector<Geometry> geometrys;
};

