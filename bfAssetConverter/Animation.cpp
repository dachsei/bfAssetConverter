#include "Animation.h"
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

using namespace Utils;

Animation::Animation(std::istream& stream, const Skeleton& skeleton)
	:skeleton(skeleton)
{
	readBinary(stream, &version);

	readBinary(stream, &boneCount);
	std::vector<uint16_t> boneIds(boneCount);
	readBinaryArray(stream, boneIds.data(), boneCount);

	readBinary(stream, &frameCount);
	readBinary(stream, &precision);

	boneAnimations.reserve(boneCount);
	for (int i = 0; i < boneCount; ++i) {
		boneAnimations.push_back(readBoneData(stream, boneIds[i]));
	}
}

void Animation::writeToCollada(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root) const
{
	char* keyframes = allocateAndComputeKeyframes(doc);
	char* interpolationValues = allocateAndFillInterpolation(doc);

	using namespace rapidxml;
	xml_node<>* libraryAnimations = root->first_node("library_animations");
	
	for (const BoneData& it : boneAnimations) {
		const std::string& boneName = skeleton.bones[it.boneId].name;
		xml_node<>* anim = doc.allocate_node(node_element, "animation");
		setId(doc, anim, boneName + "_anim");
		{
			char* inputId = writeSourceNode(doc, anim, boneName + "_anim-input", keyframes, frameCount, Format::time);
			char* matrixStream = allocateAndComputeMatrixStream(doc, it.positionStream, it.rotationStream);
			char* outputId = writeSourceNode(doc, anim, boneName + "_anim-output", matrixStream, frameCount, Format::transform);
			char* interpolationId = writeSourceNode(doc, anim, boneName + "_anim-interpolation", interpolationValues, frameCount, Format::interpolation);

			xml_node<>* sampler = doc.allocate_node(node_element, "sampler");
			char* samplerId = setId(doc, sampler, boneName + "_anim-sampler");
			{
				xml_node<>* input = doc.allocate_node(node_element, "input");
				input->append_attribute(doc.allocate_attribute("semantic", "INPUT"));
				input->append_attribute(doc.allocate_attribute("source", inputId));
				sampler->append_node(input);
				xml_node<>* output = doc.allocate_node(node_element, "input");
				output->append_attribute(doc.allocate_attribute("semantic", "OUTPUT"));
				output->append_attribute(doc.allocate_attribute("source", outputId));
				sampler->append_node(output);
				xml_node<>* interpolation = doc.allocate_node(node_element, "input");
				interpolation->append_attribute(doc.allocate_attribute("semantic", "INTERPOLATION"));
				interpolation->append_attribute(doc.allocate_attribute("source", interpolationId));
				sampler->append_node(interpolation);
			}
			anim->append_node(sampler);

			xml_node<>* channel = doc.allocate_node(node_element, "channel");
			{
				channel->append_attribute(doc.allocate_attribute("source", samplerId));
				char* target = doc.allocate_string((boneName + "/transform").c_str());
				channel->append_attribute(doc.allocate_attribute("target", target));
			}
			anim->append_node(channel);
		}
		libraryAnimations->append_node(anim);
	}
}

Animation::BoneData Animation::readBoneData(std::istream& stream, uint16_t boneId) const
{
	BoneData result;
	result.boneId = boneId;
	uint16_t datasize;
	readBinary(stream, &datasize);

	result.rotationStream.resize(frameCount);
	result.positionStream.resize(frameCount);

	for (int component = 0; component < 7; ++component) {	//7 datastreams
		size_t curFrame = 0;

		uint16_t dataLeft;
		readBinary(stream, &dataLeft);
		while (dataLeft > 0) {
			uint8_t head;
			readBinary(stream, &head);
			bool rle = (head & 0x80) != 0;		//MSB is RLE compression flag
			int numFrames = head & 0x7f;		//remaining is frame number

			uint8_t nextHeader;
			readBinary(stream, &nextHeader);
			int16_t value;

			if (rle) {
				readBinary(stream, &value);		//outside loop
			}

			for (int frame = 0; frame < numFrames; ++frame) {
				if (!rle) {
					readBinary(stream, &value);		//inside loop
				}

				switch (component) {
					case 0: result.rotationStream[curFrame].x = fixedToFloat(value); break;
					case 1: result.rotationStream[curFrame].y = fixedToFloat(value); break;
					case 2: result.rotationStream[curFrame].z = fixedToFloat(value); break;
					case 3: result.rotationStream[curFrame].w = fixedToFloat(value); break;

					case 4: result.positionStream[curFrame].x = fixedToFloat(value, precision); break;
					case 5: result.positionStream[curFrame].y = fixedToFloat(value, precision); break;
					case 6: result.positionStream[curFrame].z = fixedToFloat(value, precision); break;
				}
				++curFrame;
			}

			dataLeft -= nextHeader;
		}
	}
	for (glm::quat& rot : result.rotationStream) {
		rot = glm::inverse(rot);
	}

	return result;
}

char* Animation::allocateAndComputeMatrixStream(rapidxml::xml_document<>& doc, const std::vector<glm::vec3>& positionStream, const std::vector<glm::quat>& rotationStream) const
{
	std::stringstream ss;
	for (size_t i = 0; i < positionStream.size(); ++i) {
		glm::mat4 localMat = glm::translate(glm::mat4(), positionStream[i]) * glm::mat4_cast(rotationStream[i]);
		writeMatrixToStream(ss, localMat);
	}

	std::string result = ss.str();
	result.pop_back();
	return doc.allocate_string(result.c_str(), result.length() + 1);
}

char* Animation::allocateAndComputeKeyframes(rapidxml::xml_document<>& doc) const
{
	std::stringstream ss;
	constexpr float dt = 1.0f/30.0f;
	float time = 0.0f;

	for (size_t i = 0; i < frameCount; ++i) {
		ss << time << " ";
		time += dt;
	}

	std::string result = ss.str();
	result.pop_back();
	return doc.allocate_string(result.c_str(), result.length() +1);
}

char* Animation::allocateAndFillInterpolation(rapidxml::xml_document<>& doc) const
{
	std::stringstream ss;

	for (size_t i = 0; i < frameCount; ++i) {
		ss << "LINEAR ";
	}

	std::string result = ss.str();
	result.pop_back();
	return doc.allocate_string(result.c_str(), result.length() +1);
}
