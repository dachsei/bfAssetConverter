#include "Skeleton.h"
#include <string>

using namespace Utils;
using namespace rapidxml;

Skeleton::Skeleton(std::istream & stream)
{
	readBinary(stream, &version);
	if (version != 2)
		throw std::runtime_error("Version is not supported");

	readBinary(stream, &boneCount);
	bones.resize(boneCount);
	for (uint32_t i = 0; i < boneCount; ++i) {
		bones[i] = readBone(stream);
	}
}

void Skeleton::writeToCollada(xml_document<>& doc, xml_node<> *root) const
{
	std::vector<xml_node<>*> parents(boneCount);

	xml_node<> *armature = createArmatureNode(doc, root);

	for (size_t i = 0; i < boneCount; i++) {	//Parents are always before children
		xml_node<> *node = doc.allocate_node(node_element, "node");
		const Bone& bone = bones[i];
		parents[i] = node;

		char* boneName = doc.allocate_string(bone.name.c_str(), bone.name.length() + 1);
		node->append_attribute(doc.allocate_attribute("id", boneName));
		node->append_attribute(doc.allocate_attribute("name", boneName));
		node->append_attribute(doc.allocate_attribute("sid", boneName));
		node->append_attribute(doc.allocate_attribute("type", "JOINT"));

		char *matrixData = doc.allocate_string(formatMatrix(bone.position, bone.rotation).c_str());
		xml_node<> *matrix = doc.allocate_node(node_element, "matrix", matrixData);
		matrix->append_attribute(doc.allocate_attribute("sid", "transform"));
		node->append_node(matrix);

		if (bone.parent == -1) {
			armature->append_node(node);
		}
		else {
			parents[bone.parent]->append_node(node);
		}
	}
}

Skeleton::Bone Skeleton::readBone(std::istream& stream) const
{
	Bone bone;
	uint16_t nameLength;
	readBinary(stream, &nameLength);
	bone.name = readString(stream, nameLength);

	readBinary(stream, &bone.parent);
	readBinary(stream, &bone.rotation);
	bone.rotation = glm::inverse(bone.rotation);
	readBinary(stream, &bone.position);

	return bone;
}

xml_node<>* Skeleton::createArmatureNode(xml_document<>& doc, xml_node<> *root) const
{
	xml_node<> *visualScene = root->first_node("library_visual_scenes")->first_node("visual_scene");

	xml_node<> *armatureNode = doc.allocate_node(node_element, "node");
	visualScene->append_node(armatureNode);
	armatureNode->append_attribute(doc.allocate_attribute("id", "Armature"));
	armatureNode->append_attribute(doc.allocate_attribute("name", "Armature"));
	armatureNode->append_attribute(doc.allocate_attribute("type", "NODE"));

	xml_node<> *matrix = doc.allocate_node(node_element, "matrix");
	armatureNode->append_node(matrix);
	matrix->append_attribute(doc.allocate_attribute("sid", "transform"));

	return armatureNode;
}