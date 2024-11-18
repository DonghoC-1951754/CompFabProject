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
private:
	void setMeshToCorrectPos(Mesh* mesh);
};

