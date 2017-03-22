#include <iostream>
#include <fstream>
#include <rapidxml/rapidxml_print.hpp>
#include "Utils.h"
#include "Skeleton.h"
#include "Animation.h"

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cout << "The .ske file must be specified as parameter" << std::endl;
		return -1;
	}

	std::ifstream skeFile { argv[1], std::ifstream::in | std::ifstream::binary };
	if (!skeFile.good()) {
		std::cout << "Could not open skeleton file " << argv[1] << std::endl;
		return -1;
	}

	Skeleton skeleton{ skeFile };
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<>* root = Utils::createColladaFramework(doc);
	skeleton.writeToCollada(doc, root);

	if (argc >= 3) {
		std::ifstream animationFile{ argv[2], std::ifstream::in | std::ifstream::binary };
		if (!animationFile.good()) {
			std::cout << "Could not open animation file " << argv[2] << std::endl;
			return -1;
		}

		Animation animation{ animationFile, skeleton };
		animation.writeToCollada(doc, root);
	}

	std::ofstream output{ "C:/Users/phili/Desktop/asdf.dae" };
	output << doc;

	std::cin.ignore();

	return 0;
}