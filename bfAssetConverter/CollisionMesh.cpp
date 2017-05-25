#include "CollisionMesh.h"

using namespace Utils;

CollisionMesh::CollisionMesh(std::istream& stream)
{
	stream.ignore(4);
	readBinary(stream, &version);

	uint32_t geomCount;
	readBinary(stream, &geomCount);
	geometrys.resize(geomCount);
	for (Geometry& geom : geometrys) {
		ReadGeometry(stream, geom);
	}
}

void CollisionMesh::ReadGeometry(std::istream& stream, Geometry& geom) const
{
	uint32_t subCount;
	readBinary(stream, &subCount);
	geom.subGeoms.resize(subCount);
	for (SubGeometry& subGeom : geom.subGeoms) {
		ReadSubGeometry(stream, subGeom);
	}
}

void CollisionMesh::ReadSubGeometry(std::istream& stream, SubGeometry& geom) const
{
	uint32_t lodCount;
	readBinary(stream, &lodCount);
	geom.lods.resize(lodCount);
	for (Lod& lod : geom.lods) {
		ReadLod(stream, lod);
	}
}

void CollisionMesh::ReadLod(std::istream& stream, Lod& lod) const
{
	if (version >= 9)
		readBinary(stream, &lod.coltype);

	uint32_t faceCount;
	readBinary(stream, &faceCount);
	lod.faces.resize(faceCount);
	readBinaryArray(stream, lod.faces.data(), faceCount);

	uint32_t vertexCount;
	readBinary(stream, &vertexCount);
	lod.vertices.resize(vertexCount);
	readBinaryArray(stream, lod.vertices.data(), vertexCount);
	lod.vertexIds.resize(vertexCount);
	readBinaryArray(stream, lod.vertexIds.data(), vertexCount);

	readBinary(stream, &lod.min);
	readBinary(stream, &lod.max);

	stream.ignore(1);
	readBinary(stream, &lod.bmin);
	readBinary(stream, &lod.bmax);

	uint32_t unknownCount;
	readBinary(stream, &unknownCount);
	stream.ignore(unknownCount * 16);

	uint32_t unknownCount2;
	readBinary(stream, &unknownCount2);
	stream.ignore(unknownCount2 * 2);

	if (version >= 10) {
		uint32_t unknownCount3;
		readBinary(stream, &unknownCount3);
		stream.ignore(unknownCount3 * 4);
	}
}
