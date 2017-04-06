#include <iostream>
#include <fstream>
#include <map>
#include <tclap/CmdLine.h>

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
	stream.ignore(1);	//Eat newline
}
