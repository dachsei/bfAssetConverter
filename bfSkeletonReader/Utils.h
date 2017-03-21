#pragma once
#include <istream>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <rapidxml/rapidxml.hpp>

namespace Utils {
	template<typename T> void readBinary(std::istream& stream, T *result)
	{
		stream.read(reinterpret_cast<char*>(result), sizeof(T));
	}
	// result can only be null if elementCount is 0
	template<typename T> void readBinaryArray(std::istream& stream, T *result, size_t elementCount)
	{
		if(elementCount)
			stream.read(reinterpret_cast<char*>(result), sizeof(T) * elementCount);
	}
	std::string readString(std::istream& stream, size_t length);
	std::string readStringFormat2(std::istream& stream);

	float decompFloat(int16_t value, uint8_t precision = 15);
	std::string formatMatrix(glm::vec3 pos, glm::quat rot);
	char* allocateAndComputeMatrixStream(rapidxml::xml_document<>& doc, const std::vector<glm::vec3>& positionStream, const std::vector<glm::quat>& rotationStream);
	void writeMatrixToStream(std::ostream& stream, const glm::mat4& mat);
	char* floatsToString(rapidxml::xml_document<>& doc, const std::vector<float>& data);
	char* indicesToString(rapidxml::xml_document<>& doc, const std::vector<size_t>& data);
	enum class Format { xyz, st, weight, transform, joint };
	char* writeSourceNode(rapidxml::xml_document<>& doc, rapidxml::xml_node<> *parent, const std::string& id, const char *dataString, size_t elemCount, Format format);

	// Sets id attribute to (baseName + specifer), returns #<id> to reference this node
	template<size_t length> char *setId(rapidxml::xml_document<>& doc, rapidxml::xml_node<> *node, const std::string& baseName, const char(&specifier)[length])
	{
		size_t baseLength = baseName.length();
		//TODO: remove \0 code, because specifier already has \0
		char *id = doc.allocate_string(nullptr, baseLength + length + 2);	//leading # and ending \0
		id[0] = '#';
		memcpy(id + 1, baseName.data(), baseLength);
		memcpy(id + baseLength + 1, specifier, length);
		id[baseLength + length + 1] = '\0';

		node->append_attribute(doc.allocate_attribute("id", id + 1, sizeof("id")-1, baseLength + length - 1));	//without #
		return id;
	}
	// Sets id attribute, returns #<id> to reference this node
	char* setId(rapidxml::xml_document<>& doc, rapidxml::xml_node<> *node, const std::string& id);

	rapidxml::xml_node<> *createColladaFramework(rapidxml::xml_document<>& doc);
}