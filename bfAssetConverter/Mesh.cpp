#include "Mesh.h"

using namespace Utils;

Mesh::Mesh(std::istream& stream)
{
	stream.ignore(1 * 4);	//unused
	readBinary(stream, &version);
	stream.ignore(3 * 4);

	//Geometry table
	stream.ignore(1);
	uint32_t geomCount;
	readBinary(stream, &geomCount);
	geometrys.resize(geomCount);
	for (Geometry& it : geometrys) {
		uint32_t lodCount;
		readBinary(stream, &lodCount);
		it.lods.resize(lodCount);
	}

	//Vertex attribute table
	uint32_t vertexAttributeCount;
	readBinary(stream, &vertexAttributeCount);
	vertexAttribs.resize(vertexAttributeCount);
	readBinaryArray(stream, vertexAttribs.data(), vertexAttributeCount);

	//Vertices
	readBinary(stream, &vertexformat);
	readBinary(stream, &vertexstride);
	uint32_t vertexCount;
	readBinary(stream, &vertexCount);
	vertices.resize((vertexstride / vertexformat)*vertexCount);
	readBinaryArray(stream, vertices.data(), vertices.size());

	//Indices
	uint32_t indexCount;
	readBinary(stream, &indexCount);
	indices.resize(indexCount);
	readBinaryArray(stream, indices.data(), indices.size());
}

void Mesh::readMaterial(std::istream& stream, Material& material) const
{
	material.fxFile = readStringFormat2(stream);
	material.technique = readStringFormat2(stream);

	uint32_t mappingCount;
	readBinary(stream, &mappingCount);
	material.map.resize(mappingCount);
	for (std::string& name : material.map) {
		name = readStringFormat2(stream);
	}

	readBinary(stream, &material.vertexOffset);
	readBinary(stream, &material.indexOffset);
	readBinary(stream, &material.indexCount);
	readBinary(stream, &material.vertexCount);

	stream.ignore(2 * 4);
}

void Mesh::flipTextureCoords()
{
	size_t offset = -1;
	for (const VertexAttrib& attrib : vertexAttribs) {
		if (attrib.usage == VertexAttrib::uv1) {
			offset = attrib.offset / vertexformat;
			assert(attrib.vartype == VertexAttrib::float2);
		}
	}
	assert(offset != -1);

	for (size_t i = offset + 1; i < vertices.size(); i += vertexstride / vertexformat) {	//+1 for the y component
		vertices[i] = 1 - vertices[i];
	}
}
