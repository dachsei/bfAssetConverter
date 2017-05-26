#include "CollisionMesh.h"
#include <iostream>
#include <sstream>
#include <array>
#include <memory>
#include <fstream>
#include <rapidxml/rapidxml_print.hpp>

using namespace Utils;
using namespace rapidxml;

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

void CollisionMesh::writeFiles(const std::string& baseName) const
{
	std::array<SimpleIndexedGeometry, 3> tmpGeometries;

	for (const Geometry& geom : geometrys) {
		for (const SubGeometry& sub : geom.subGeoms) {
			for (const Lod& lod : sub.lods) {
				for (const Face& face : lod.faces) {
					//Reverse Vertices
					tmpGeometries[lod.coltype].addVertex(lod.vertices[face.v3]);
					tmpGeometries[lod.coltype].addVertex(lod.vertices[face.v2]);
					tmpGeometries[lod.coltype].addVertex(lod.vertices[face.v1]);
				}
			}
		}
	}

	WriteSimpleGeometry(baseName + "_projectile" + ".dae", tmpGeometries[0]);
	WriteSimpleGeometry(baseName + "_vehicle" + ".dae", tmpGeometries[1]);
	WriteSimpleGeometry(baseName + "_soldier" + ".dae", tmpGeometries[2]);
}

void CollisionMesh::WriteSimpleGeometry(const std::string& name, const SimpleIndexedGeometry& geometry) const
{
	auto doc = std::make_unique<rapidxml::xml_document<>>();
	xml_node<>* root = Utils::createColladaFramework(*doc);
	
	xml_node<>* geometryNode = doc->allocate_node(node_element, "geometry");
	char* meshId = setId(*doc, geometryNode, "Object-mesh");
	{
		xml_node<>* mesh = doc->allocate_node(node_element, "mesh");
		{
			char* positionData = Utils::floatsToString(*doc, geometry.vertices);
			char* positionsId = writeSourceNode(*doc, mesh, "Object-mesh-positions", positionData, geometry.vertices.size() / 3, Format::xyz);

			xml_node<>* vertices = doc->allocate_node(node_element, "vertices");
			char* verticesId = setId(*doc, vertices, "Object-mesh-vertices");
			{
				xml_node<>* input = doc->allocate_node(node_element, "input");
				input->append_attribute(doc->allocate_attribute("semantic", "POSITION"));
				input->append_attribute(doc->allocate_attribute("source", positionsId));
				vertices->append_node(input);
			}
			mesh->append_node(vertices);

			xml_node<>* polylist = doc->allocate_node(node_element, "polylist");
			{
				char* indexData = indicesToString(*doc, geometry.indices);
				polylist->append_attribute(doc->allocate_attribute("count", doc->allocate_string(
					std::to_string(geometry.indices.size() / 3).c_str()
				)));

				xml_node<> *input = doc->allocate_node(node_element, "input");
				input->append_attribute(doc->allocate_attribute("semantic", "VERTEX"));
				input->append_attribute(doc->allocate_attribute("source", verticesId));
				input->append_attribute(doc->allocate_attribute("offset", "0"));
				polylist->append_node(input);

				std::stringstream ss;
				for (size_t i = 0; i < geometry.indices.size() / 3; ++i) {
					ss << "3 ";
				}
				std::string result = ss.str();
				result.pop_back();
				char* vcountData = doc->allocate_string(result.c_str(), result.length() + 1);

				polylist->append_node(doc->allocate_node(node_element, "vcount", vcountData));
				polylist->append_node(doc->allocate_node(node_element, "p", indexData));
			}
			mesh->append_node(polylist);
		}
		geometryNode->append_node(mesh);
	}
	root->first_node("library_geometries")->append_node(geometryNode);
	
	xml_node<>* node = doc->allocate_node(node_element, "node");
	{
		char* id = setId(*doc, node, "Object");
		node->append_attribute(doc->allocate_attribute("name", id + 1));
		node->append_attribute(doc->allocate_attribute("type", "NODE"));

		xml_node<>* instanceGeometry = doc->allocate_node(node_element, "instance_geometry");
		{
			instanceGeometry->append_attribute(doc->allocate_attribute("url", meshId));
			instanceGeometry->append_attribute(doc->allocate_attribute("name", id + 1));
		}
		node->append_node(instanceGeometry);
	}
	root->first_node("library_visual_scenes")->first_node("visual_scene")->append_node(node);

	std::ofstream output{ name };
	if (!output.good())
		throw Utils::ConversionError("Can not write to output file " + name);
	output << *doc;
	std::cout << "   -->" << name << std::endl;
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
	for (size_t i = 0; i < lod.vertices.size(); i += 3) {
		lod.vertices[i] = -lod.vertices[i];
	}
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

void CollisionMesh::SimpleIndexedGeometry::addVertex(glm::vec3 vertex)
{
	auto ins = indexMap.insert(std::make_pair(vertex, vertices.size()));
	if (ins.second) {
		vertices.push_back(vertex.x);
		vertices.push_back(vertex.y);
		vertices.push_back(vertex.z);
	}
	indices.push_back(ins.first->second / 3);
}

bool std::operator<(const glm::vec3& lhs, const glm::vec3& rhs)
{
	return std::tie(lhs.x, lhs.y, lhs.z) < std::tie(rhs.x, rhs.y, rhs.z);
}