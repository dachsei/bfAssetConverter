#pragma once
#include "Mesh.h"

class BundledMesh : public Mesh
{
public:
	BundledMesh(std::istream& stream);
	~BundledMesh() = default;
};
