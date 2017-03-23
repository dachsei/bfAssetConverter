# bfAssetConverter
Command line utility for converting Battlefield Heroes assets to common formats
based on [BfMeshView](https://github.com/ByteHazard/BfMeshView)

# Usage
bfAssetConverter.exe <filename> [-o <filename>] [-s <filename>]

Where:
* <filename> (accepted multiple times) Files to convert
* -o <filename>, --output <filename> (accepted multiple times) Output files
* -s <filename>, --skeleton <filename> Skeleton file (.ske)

Avoid bfAssetConverter.exe in1 in2 -o out2 because it converts in1 -> out2, and in2 -> defaultOutput(in2)

Animations (.baf) and SkinnedMeshes (.skinnedmesh) require a skeleton file

# Dependencies
* [Templatized C++ Command Line Parser Library](http://tclap.sourceforge.net/)
* [RapidJSON](http://rapidjson.org/)
* [GLM](http://glm.g-truc.net/0.9.8/index.html)

# License
GNU GPLv3
