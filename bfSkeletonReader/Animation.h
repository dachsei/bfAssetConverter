#pragma once
#include "Skeleton.h"

class Animation
{
public:
	Animation(std::istream& stream, const Skeleton& skeleton);
	~Animation() = default;

	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const;

private:
	struct BoneFrame {
		glm::quat rotation;
		glm::vec3 position;
	};
	struct BoneData {
		uint16_t boneId;
		std::vector<glm::quat> rotationStream;
		std::vector<glm::vec3> positionStream;
	};

	BoneData readBoneData(std::istream& stream, uint16_t boneId) const;
	char* allocateAndComputeMatrixStream(rapidxml::xml_document<>& doc, const std::vector<glm::vec3>& positionStream, const std::vector<glm::quat>& rotationStream) const;
	char* allocateAndComputeKeyframes(rapidxml::xml_document<>& doc) const;
	char* allocateAndFillInterpolation(rapidxml::xml_document<>& doc) const;

	const Skeleton& skeleton;
	uint32_t version;
	uint16_t boneCount;
	uint32_t frameCount;
	uint8_t precision;
	std::vector<BoneData> boneAnimations;
};

