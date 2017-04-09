#include <iostream>
#include <fstream>
#include <memory>
#include <rapidxml/rapidxml_print.hpp>
#include <tclap/CmdLine.h>
#include "Utils.h"
#include "Animation.h"
#include "SkinnedMesh.h"
#include "BundledMesh.h"

std::string getExtension(const std::string& filename);
std::string defaultOutputFile(const std::string& filename);
void convertFile(std::istream& input, std::ostream& output, const std::string& extension, const Skeleton* skeleton);

int main(int argc, char** argv)
{
	try {
		TCLAP::CmdLine cmd{ "Converts Battlefield assets to common formats", ' ', "1.0" };
		TCLAP::ValueArg<std::string> skeletonArg{ "s", "skeleton", "Skeleton file (.ske)", false, "", "filename", cmd };
		TCLAP::UnlabeledMultiArg<std::string> fileArgs{ "filenames", "Files to convert", true, "filename", cmd };
		TCLAP::MultiArg<std::string> outputArgs{ "o", "output", "Output files (same order as input files)", false, "filename", cmd };
		
		cmd.parse(argc, argv);

		std::unique_ptr<Skeleton> skeleton;
		if (skeletonArg.isSet()) {
			std::ifstream skeletonFile{ skeletonArg.getValue(), std::ifstream::in | std::ifstream::binary };
			if(!skeletonFile.good())
				throw std::runtime_error("Could not open skeleton file " + skeletonArg.getValue());
			skeleton = std::make_unique<Skeleton>(skeletonFile);
		}
		
		for (size_t i = 0; i < fileArgs.getValue().size(); ++i) {
			std::string inputName = fileArgs.getValue()[i];
			bool outputSpecified = i < outputArgs.getValue().size();
			std::string outputName = outputSpecified ? outputArgs.getValue()[i] : defaultOutputFile(inputName);

			try {
				std::ifstream inputFile{ inputName, std::ifstream::in | std::ifstream::binary };
				if (!inputFile.good())
					throw Utils::ConversionError("Could not open input");

				std::ofstream outputFile{ outputName };
				if(!outputFile.good())
					throw Utils::ConversionError("Can not write to output file " + outputName);

				convertFile(inputFile, outputFile, getExtension(inputName), skeleton.get());
				std::cout << "Converted " << inputName << " to " << outputName << std::endl;
			}
			catch (Utils::ConversionError& e) {
				std::cerr << "Error at file " << inputName << ": " << e.what() << std::endl;
			}
		}
	}
	catch (TCLAP::ArgException& e) {
		std::cerr << "error: " << e.error() << " at arg " << e.argId() << std::endl;
		return -1;
	}
	catch (std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

std::string getExtension(const std::string& filename)
{
	size_t pos = filename.find_last_of('.');
	if (pos == std::string::npos)
		return "";
	return filename.substr(pos + 1);
}

std::string defaultOutputFile(const std::string& filename)
{
	size_t pos = filename.find_last_of('.');
	if (pos == std::string::npos)
		pos = filename.length();
	return filename.substr(0, pos) + ".dae";
}

void convertFile(std::istream& input, std::ostream& output, const std::string& extension, const Skeleton* skeleton)
{
	auto doc = std::make_unique<rapidxml::xml_document<>>();
	rapidxml::xml_node<>* root = Utils::createColladaFramework(*doc);

	if(extension.compare("baf") == 0) {
		if (!skeleton)
			throw Utils::ConversionError("Animations require a skeleton file");
		Animation anim{ input, *skeleton };
		skeleton->writeToCollada(*doc, root);
		anim.writeToCollada(*doc, root);
	}
	else if (extension.compare("skinnedmesh") == 0) {
		if (!skeleton)
			throw Utils::ConversionError("Skinnedmeshes require a skeleton file");
		SkinnedMesh mesh{ input, *skeleton };
		skeleton->writeToCollada(*doc, root);
		mesh.writeToCollada(*doc, root);
	}
	else if (extension.compare("bundledmesh") == 0) {
		BundledMesh mesh{ input };
		mesh.writeToCollada(*doc, root);
	}
	else {
		throw Utils::ConversionError("Unsupported filetype " + extension);
	}
	output << *doc;
}
