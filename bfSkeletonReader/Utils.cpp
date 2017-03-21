#include "Utils.h"
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

using namespace rapidxml;

std::string Utils::readString(std::istream & stream, size_t length)
{
	std::vector<char> buffer(length);
	stream.read(buffer.data(), length);
	return std::string(buffer.data());
}

std::string Utils::readStringFormat2(std::istream& stream)
{
	uint32_t strLength;
	readBinary(stream, &strLength);
	std::vector<char> buffer(strLength);
	readBinaryArray(stream, buffer.data(), buffer.size());
	return std::string(buffer.begin(), buffer.end());
}

float Utils::decompFloat(int16_t value, uint8_t precision)
{
	float multiplicator = 1.0f / (1 << precision);
	return value * multiplicator;
}

std::string Utils::formatMatrix(glm::vec3 pos, glm::quat rot)
{
	std::stringstream ss;
	glm::mat4 localMat = glm::translate(glm::mat4_cast(rot), pos);
	localMat = glm::inverse(localMat);

	writeMatrixToStream(ss, localMat);
	std::string result = ss.str();
	result.pop_back();
	return result;
}

char *Utils::allocateAndComputeMatrixStream(rapidxml::xml_document<>& doc, const std::vector<glm::vec3>& positionStream, const std::vector<glm::quat>& rotationStream)
{
	std::stringstream ss;
	for (size_t i = 0; i < positionStream.size(); i++) {
		glm::mat4 localMat = glm::translate(glm::mat4_cast(rotationStream[i]), positionStream[i]);
		localMat = glm::inverse(localMat);
		writeMatrixToStream(ss, localMat);
	}

	std::string result = ss.str();
	result.pop_back();
	return doc.allocate_string(result.c_str(), result.length() +1);
}

void Utils::writeMatrixToStream(std::ostream& stream, const glm::mat4& mat)
{
	for (int k = 0; k < 4; k++) {
		for (int l = 0; l < 4; l++) {
			stream << mat[l][k] << " ";	//column major -> row major
		}
	}
}

char* Utils::floatsToString(rapidxml::xml_document<>& doc, const std::vector<float>& data)
{
	std::stringstream ss;
	for (const float f : data) {
		ss << f << " ";
	}
	std::string dataString = ss.str();
	dataString.pop_back();
	return doc.allocate_string(dataString.c_str(), dataString.length() + 1);
}

char * Utils::indicesToString(rapidxml::xml_document<>& doc, const std::vector<size_t>& data)
{
	std::stringstream ss;
	for (const float f : data) {
		ss << f << " ";
	}
	std::string dataString = ss.str();
	dataString.pop_back();
	return doc.allocate_string(dataString.c_str(), dataString.length() + 1);
}

char* Utils::writeSourceNode(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* parent, const std::string& id, const char *dataString, size_t elemCount, Format format)
{
	using namespace rapidxml;
	xml_node<> *source = doc.allocate_node(node_element, "source");
	char *resultId = setId(doc, source, id);
	{
		xml_node<> *arrayNode;
		switch (format)
		{
		case Format::xyz:
			arrayNode = doc.allocate_node(node_element, "float_array", dataString);
			arrayNode->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(elemCount * 3).c_str())));
			break;
		case Format::st:
			arrayNode = doc.allocate_node(node_element, "float_array", dataString);
			arrayNode->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(elemCount * 2).c_str())));
			break;
		case Format::weight:
			arrayNode = doc.allocate_node(node_element, "float_array", dataString);
			arrayNode->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(elemCount).c_str())));
			break;
		case Format::transform:
			arrayNode = doc.allocate_node(node_element, "float_array", dataString);
			arrayNode->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(elemCount * 16).c_str())));
			break;
		case Format::joint:
			arrayNode = doc.allocate_node(node_element, "Name_array", dataString);
			arrayNode->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(elemCount).c_str())));
			break;
		}
		char *dataId = setId(doc, arrayNode, id, "-array");
		source->append_node(arrayNode);

		xml_node<> *technique = doc.allocate_node(node_element, "technique_common");
		{
			xml_node<> *accessor = doc.allocate_node(node_element, "accessor");
			{
				accessor->append_attribute(doc.allocate_attribute("source", dataId));
				accessor->append_attribute(doc.allocate_attribute("count", doc.allocate_string(std::to_string(elemCount).c_str())));

				xml_node<> *param;
				switch (format) {
				case Format::xyz:
					accessor->append_attribute(doc.allocate_attribute("stride", "3"));

					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "X"));
					param->append_attribute(doc.allocate_attribute("type", "float"));
					accessor->append_node(param);
					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "Y"));
					param->append_attribute(doc.allocate_attribute("type", "float"));
					accessor->append_node(param);
					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "Z"));
					param->append_attribute(doc.allocate_attribute("type", "float"));
					accessor->append_node(param);
					break;
				case Format::st:
					accessor->append_attribute(doc.allocate_attribute("stride", "2"));

					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "S"));
					param->append_attribute(doc.allocate_attribute("type", "float"));
					accessor->append_node(param);
					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "T"));
					param->append_attribute(doc.allocate_attribute("type", "float"));
					accessor->append_node(param);
					break;
				case Format::weight:
					accessor->append_attribute(doc.allocate_attribute("stride", "1"));

					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "WEIGHT"));
					param->append_attribute(doc.allocate_attribute("type", "float"));
					accessor->append_node(param);
					break;
				case Format::transform:
					accessor->append_attribute(doc.allocate_attribute("stride", "16"));

					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "TRANSFORM"));
					param->append_attribute(doc.allocate_attribute("type", "float4x4"));
					accessor->append_node(param);
					break;
				case Format::joint:
					accessor->append_attribute(doc.allocate_attribute("stride", "1"));

					param = doc.allocate_node(node_element, "param");
					param->append_attribute(doc.allocate_attribute("name", "JOINT"));
					param->append_attribute(doc.allocate_attribute("type", "name"));
					accessor->append_node(param);
					break;
				}
			}
			technique->append_node(accessor);
		}
		source->append_node(technique);
	}
	parent->append_node(source);
	return resultId;
}

char* Utils::setId(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const std::string& id)
{
	size_t length = id.length();
	char *data = doc.allocate_string(nullptr, length + 2);	//leading # and ending \0
	data[0] = '#';
	memcpy(data + 1, id.c_str(), length + 1);
	node->append_attribute(doc.allocate_attribute("id", data + 1, sizeof("id") - 1, length));	//without #
	return data;
}

rapidxml::xml_node<>* Utils::createColladaFramework(rapidxml::xml_document<>& doc)
{
	xml_node<> *decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<> *root = doc.allocate_node(node_element, "COLLADA");
	root->append_attribute(doc.allocate_attribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema"));
	root->append_attribute(doc.allocate_attribute("version", "1.4.1"));
	doc.append_node(root);

	xml_node<> *asset = doc.allocate_node(node_element, "asset");
	xml_node<> *unit = doc.allocate_node(node_element, "unit");
	unit->append_attribute(doc.allocate_attribute("name", "meter"));
	unit->append_attribute(doc.allocate_attribute("meter", "1"));
	asset->append_node(unit);
	asset->append_node(doc.allocate_node(node_element, "up_axis", "Z_UP"));
	root->append_node(asset);

	root->append_node(doc.allocate_node(node_element, "library_images"));
	root->append_node(doc.allocate_node(node_element, "library_geometries"));
	root->append_node(doc.allocate_node(node_element, "library_animations"));
	root->append_node(doc.allocate_node(node_element, "library_controllers"));

	xml_node<> *libraryVisualScene = doc.allocate_node(node_element, "library_visual_scenes");
	{
		xml_node<> *visualScene = doc.allocate_node(node_element, "visual_scene");
		visualScene->append_attribute(doc.allocate_attribute("id", "Scene"));
		visualScene->append_attribute(doc.allocate_attribute("name", "Scene"));
		libraryVisualScene->append_node(visualScene);
	}
	root->append_node(libraryVisualScene);

	return root;
}
