#pragma once
#include "Utils.h"

class Skeleton
{
public:
	Skeleton(std::istream& stream);
	~Skeleton() = default;

	void writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const;

private:
	struct Bone {
		std::string name;
		int16_t parent;
		glm::quat rotation;
		glm::vec3 position;
	};

	Bone readBone(std::istream& stream) const;
	rapidxml::xml_node<>* Skeleton::createArmatureNode(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const;

	uint32_t version;
	std::vector<Bone> bones;

	friend class Animation;
	friend class SkinnedMesh;
};