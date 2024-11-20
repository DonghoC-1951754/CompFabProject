#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <iostream>
#include "Vertex.h"
#include "Mesh.h"


class ObjectLoader
{
public:
	ObjectLoader();
	Mesh* loadSTL(const std::string& filename);
	void setPlateWidth(double width) { plateWidth = width;};
	void setPlateDepth(double depth) { plateDepth = depth;};

	double getPlateWidth() { return plateWidth; };
	double getPlateDepth() { return plateDepth; };
private:
	void setMeshToCorrectPos(Mesh* mesh);
	double plateWidth = 180.0;
	double plateDepth = 180.0;
};

