#include <iostream>
#include <fstream>
#include <tclap/CmdLine.h>

struct ConFile {
	std::string textureFile;
	enum {
		Body, Hands, Legs, Feet, Head, HeadHair, Hat, Hair, Neck, FaceObj, FaceHair, WaistLeft, WaistRight, WaistBack, ChestItem
	} itemType;
	int xOffset;
	int yOffset;
};

ConFile parseConFile(std::istream& stream, const std::string& overrideFolder);
void processFile(const ConFile& file);

int main(int argc, char** argv)
{
	try {
		TCLAP::CmdLine cmd{ "Resizes textures to match meshes", ' ', "1.0" };
		TCLAP::UnlabeledMultiArg<std::string> inputArgs{ "input", "Input files .con", true, "filename", cmd };
		TCLAP::ValueArg<std::string> textureFolderArg{ "t", "textureOverride", "Folder where the texture is stored", true, "", "folder", cmd };
		TCLAP::ValueArg<std::string> outputFolder{ "o", "output", "output Folder", false, "./", "folder", cmd };

		cmd.parse(argc, argv);

		for (const std::string& filename : inputArgs.getValue()) {
			try {
				std::ifstream inputStream{ filename };
				if (!inputStream.good())
					throw std::runtime_error("Could not open Con File");
				ConFile conFile = parseConFile(inputStream, textureFolderArg.getValue());

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

ConFile parseConFile(std::istream & stream, const std::string & overrideFolder)
{
	return ConFile();
}

void processFile(const ConFile & file)
{
}
