#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <tclap/CmdLine.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct ConFile {
	std::string textureFile;
	std::string outputFile;
	enum ItemType {
		Body, Hands, Legs, Feet, Head, HeadHair, Hat, Hair, Neck, FaceObj, FaceHair, WaistLeft, WaistRight, WaistBack, ChestItem
	};
	ItemType itemType;
	int xOffset;
	int yOffset;

	void readItemType(std::istream& stream);
	void readTextureFilename(std::istream& stream, const std::string& overrideFolder, const std::string& outputFolder);
	void readOffset(std::istream& stream);
};
ConFile parseConFile(std::istream& stream, const std::string& overrideFolder, const std::string& outputFolder);
void processFile(const ConFile& file);
void targetSize(ConFile::ItemType type, int& width, int& height);
void copyImageRegion(const unsigned char* src, unsigned char* dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int xOffset, int yOffset, int channels);

int main(int argc, char** argv)
{
	try {
		TCLAP::CmdLine cmd{ "Resizes textures to match meshes", ' ', "1.0" };
		TCLAP::UnlabeledMultiArg<std::string> inputArgs{ "input", "Input files .con", true, "filename", cmd };
		TCLAP::ValueArg<std::string> textureFolderArg{ "t", "textureOverride", "Folder where the texture is stored", true, "", "folder", cmd };
		TCLAP::ValueArg<std::string> outputFolder{ "o", "output", "output Folder", false, ".", "folder", cmd };

		cmd.parse(argc, argv);

		for (const std::string& filename : inputArgs.getValue()) {
			try {
				std::ifstream inputStream{ filename };
				if (!inputStream.good())
					throw std::runtime_error("Could not open Con File");

				ConFile conFile = parseConFile(inputStream, textureFolderArg.getValue(), outputFolder.getValue());
				processFile(conFile);
			}
			catch (std::runtime_error& e) {
				std::cerr << "Error at file " << filename << ": " << e.what() << std::endl;
			}
		}
	}
	catch (TCLAP::ArgException& e) {
		std::cerr << "error: " << e.error() << " at arg " << e.argId() << std::endl;
		return -1;
	}

	return 0;
}

ConFile parseConFile(std::istream& stream, const std::string& overrideFolder, const std::string& outputFolder)
{
	ConFile result;
	std::string line;
	while (std::getline(stream, line)) {
		std::stringstream ss{ line };
		std::string function;
		std::getline(ss, function, ' ');

		if (function == "geometryTemplate.itemType")
			result.readItemType(ss);
		else if (function == "geometryTemplate.fileName")
			result.readTextureFilename(ss, overrideFolder, outputFolder);
		else if (function == "geometryTemplate.setCoordinate")
			result.readOffset(ss);
	}
	return result;
}

void processFile(const ConFile& file)
{
	int srcWidth, srcHeight, dstWidth, dstHeight, channels;
	targetSize(file.itemType, dstWidth, dstHeight);

	std::unique_ptr<unsigned char, void(*)(void*)> inputData(
		stbi_load(file.textureFile.c_str(), &srcWidth, &srcHeight, &channels, 0),
		&stbi_image_free);
	std::unique_ptr<unsigned char[]> outputData(new unsigned char[channels*dstWidth*dstHeight]);

	if (srcWidth + file.xOffset > dstWidth || srcHeight + file.yOffset > dstHeight)
		throw std::runtime_error("Image writing exceeds Bounds");

	for (int y = 0; y < srcHeight; ++y) {
		memcpy(
			outputData.get() + (file.xOffset + (y+file.yOffset) *dstWidth) * channels,
			inputData.get() + y*srcWidth * channels,
			srcWidth * channels);
	}

	stbi_write_tga(file.outputFile.c_str(), dstWidth, dstHeight, channels, outputData.get());
}

void targetSize(ConFile::ItemType type, int& width, int& height)
{
	switch (type) {
	case ConFile::Body:			width = 1024;	height = 512;	break;
	case ConFile::Hands:		width = 1024;	height = 512;	break;
	case ConFile::Legs:			width = 369;	height = 296;	break;
	case ConFile::Feet:			width = 192;	height = 124;	break;
	case ConFile::Head:			width = 463;	height = 296;	break;
	case ConFile::HeadHair:		width = 463;	height = 296;	break;
	case ConFile::Hat:			width = 317;	height = 216;	break;
	case ConFile::Hair:			width = 317;	height = 216;	break;
	case ConFile::Neck:			width = 87;		height = 216;	break;
	case ConFile::FaceObj:		width = 192;	height = 86;	break;
	case ConFile::FaceHair:		width = 192;	height = 86;	break;
	case ConFile::WaistLeft:	width = 155;	height = 216;	break;
	case ConFile::WaistRight:	width = 155;	height = 216;	break;
	case ConFile::WaistBack:	width = 155;	height = 216;	break;
	case ConFile::ChestItem:	width = 155;	height = 216;	break;
	}
}

void copyImageRegion(const unsigned char* src, unsigned char* dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int xOffset, int yOffset, int channels)
{
	if (srcWidth + xOffset > dstWidth || srcHeight + yOffset > dstHeight)
		throw std::runtime_error("Image writing exceeds Bounds");

	for (int y = 0; y < srcHeight; ++y) {
		memcpy(dst + xOffset + (y+yOffset)*dstWidth, src + y*srcWidth, channels*srcWidth);
	}
}

void ConFile::readItemType(std::istream& stream)
{
	std::string buffer;
	stream >> buffer;
	if (buffer == "CIT_Body")
		itemType = Body;
	else if (buffer == "CIT_Hands")
		itemType = Hands;
	else if (buffer == "CIT_Legs")
		itemType = Legs;
	else if (buffer == "CIT_Feet")
		itemType = Feet;
	else if (buffer == "CIT_Head")
		itemType = Head;
	else if (buffer == "CIT_HeadHair")
		itemType = HeadHair;
	else if (buffer == "CIT_Hat")
		itemType = Hat;
	else if (buffer == "CIT_Hair")
		itemType = Hair;
	else if (buffer == "CIT_Neck")
		itemType = Neck;
	else if (buffer == "CIT_FaceObj")
		itemType = FaceObj;
	else if (buffer == "CIT_FaceHair")
		itemType = FaceHair;
	else if (buffer == "CIT_WaistLeft")
		itemType = WaistLeft;
	else if (buffer == "CIT_WaistRight")
		itemType = WaistRight;
	else if (buffer == "CIT_WaistBack")
		itemType = WaistBack;
	else if (buffer == "CIT_ChestItem")
		itemType = ChestItem;
	else
		throw std::runtime_error("Unknown item type " + buffer);
}

void ConFile::readTextureFilename(std::istream& stream, const std::string& overrideFolder, const std::string& outputFolder)
{
	std::string buffer;
	std::getline(stream, buffer, '\n');
	size_t pos = buffer.find_last_of('/');

	textureFile = overrideFolder + '/' + buffer.substr(pos + 1);
	outputFile = outputFolder + '/' + buffer.substr(pos + 1);
}

void ConFile::readOffset(std::istream& stream)
{
	stream >> xOffset;
	if (stream.get() != '/')
		throw std::runtime_error("Cannot parse offset");
	stream >> yOffset;
}
