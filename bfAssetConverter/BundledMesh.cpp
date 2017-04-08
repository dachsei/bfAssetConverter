#include "BundledMesh.h"

using namespace Utils;

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