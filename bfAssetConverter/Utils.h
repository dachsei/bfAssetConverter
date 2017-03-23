#pragma once
#include <istream>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <rapidxml/rapidxml.hpp>

namespace Utils {
	class ConversionError :public std::runtime_error {
	public:
		ConversionError(const std::string& msg)
			:std::runtime_error(msg) {}
	};

	template<typename T> void readBinary(std::istream& stream, T* result)
	{
		stream.read(reinterpret_cast<char*>(result), sizeof(T));
	}
	// result can only be null if elementCount is 0
	template<typename T> void readBinaryArray(std::istream& stream, T* result, size_t elementCount)
	{
		if(elementCount)
			stream.read(reinterpret_cast<char*>(result), sizeof(T) * elementCount);
	}
	std::string readStringFormat1(std::istream& stream);
	std::string readStringFormat2(std::istream& stream);

	float fixedToFloat(int16_t value, uint8_t precision = 15);
	void writeMatrixToStream(std::ostream& stream, const glm::mat4& mat);
	char* matrixToString(rapidxml::xml_document<>& doc, const glm::mat4& mat);
	char* floatsToString(rapidxml::xml_document<>& doc, const std::vector<float>& data);
	char* indicesToString(rapidxml::xml_document<>& doc, const std::vector<size_t>& data);
	enum class Format { xyz, st, weight, transform, joint, time, interpolation };
	char* writeSourceNode(rapidxml::xml_document<>& doc, rapidxml::xml_node<> *parent, const std::string& id, const char* dataString, size_t elemCount, Format format);

	// Sets id attribute, returns #<id> to reference this node
	char* setId(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const std::string& id);

	rapidxml::xml_node<>* createColladaFramework(rapidxml::xml_document<>& doc);
}