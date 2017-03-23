#include "Skeleton.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Utils;
using namespace rapidxml;

Skeleton::Skeleton(std::istream& stream) throw(Utils::ConversionError)
{
	readBinary(stream, &version);
	if (version != 2)
		throw Utils::ConversionError("Version is not supported");

	uint32_t boneCount;
	readBinary(stream, &boneCount);
	bones.resize(boneCount);
	for (Bone& bone : bones) {
		bone = readBone(stream);
	}
}

void Skeleton::writeToCollada(xml_document<>& doc, xml_node<>* root) const
{
	std::vector<xml_node<>*> parents(bones.size());

	xml_node<>* armature = createArmatureNode(doc, root);

	for (size_t i = 0; i < bones.size(); i++) {	//Parents are always before children
		xml_node<>* node = doc.allocate_node(node_element, "node");
		const Bone& bone = bones[i];
		parents[i] = node;

		char* boneName = doc.allocate_string(bone.name.c_str(), bone.name.length() + 1);
		node->append_attribute(doc.allocate_attribute("id", boneName));
		node->append_attribute(doc.allocate_attribute("name", boneName));
		node->append_attribute(doc.allocate_attribute("sid", boneName));
		node->append_attribute(doc.allocate_attribute("type", "JOINT"));

		glm::mat4 boneMat = glm::translate(glm::mat4(), bone.position) * glm::mat4_cast(bone.rotation);
		xml_node<>* matrix = doc.allocate_node(node_element, "matrix", matrixToString(doc, boneMat));
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
	bone.name = readStringFormat1(stream);

	readBinary(stream, &bone.parent);
	readBinary(stream, &bone.rotation);
	bone.rotation = glm::inverse(bone.rotation);
	readBinary(stream, &bone.position);

	return bone;
}

xml_node<>* Skeleton::createArmatureNode(xml_document<>& doc, xml_node<>* root) const
{
	xml_node<>* visualScene = root->first_node("library_visual_scenes")->first_node("visual_scene");

	xml_node<>* armatureNode = doc.allocate_node(node_element, "node");
	visualScene->append_node(armatureNode);
	armatureNode->append_attribute(doc.allocate_attribute("id", "Armature"));
	armatureNode->append_attribute(doc.allocate_attribute("name", "Armature"));
	armatureNode->append_attribute(doc.allocate_attribute("type", "NODE"));

	xml_node<>* matrix = doc.allocate_node(node_element, "matrix");
	armatureNode->append_node(matrix);
	matrix->append_attribute(doc.allocate_attribute("sid", "transform"));

	return armatureNode;
}