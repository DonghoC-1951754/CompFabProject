#include "ObjectLoader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

ObjectLoader::ObjectLoader() {
}

Mesh* ObjectLoader::loadSTL(const std::string& filename) {
    Assimp::Importer importer;
    // Load the STL file
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        //return {};
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    aiMesh* mesh = scene->mMeshes[0];

	//Lowest Z and Y get swapped because of the coordinate system
    float lowestX = std::numeric_limits<float>::max();
	float highestX = std::numeric_limits<float>::min();
    float lowestY = std::numeric_limits<float>::max();
	float highestY = std::numeric_limits<float>::min();
    float lowestZ = std::numeric_limits<float>::max();
    float highestZ = std::numeric_limits<float>::min();

    // First loop through the vertices to populate the vertices vector
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        auto aiVertices = mesh->mVertices[i];
        auto aiNormals = mesh->mNormals[i];
        Vertex vertex;
		if (aiVertices.x < lowestX) {
			lowestX = aiVertices.x;
		}
		if (aiVertices.x > highestX) {
			highestX = aiVertices.x;
		}
		if (aiVertices.z < lowestY) {
			lowestY = aiVertices.z;
		}
		if (aiVertices.z > highestY) {
			highestY = aiVertices.z;
		}
		if (aiVertices.y < lowestZ) {
			lowestZ = aiVertices.y;
		}
		if (aiVertices.y > highestZ) {
			highestZ = aiVertices.y;
		}

		//vertex.setPosition(glm::vec3(aiVertices.x, aiVertices.z, aiVertices.y));
		vertex.setPosition(glm::vec3(aiVertices.x, aiVertices.z, aiVertices.y));// x = x, y = z: hoogte, z = y: diepte
        vertices.push_back(vertex);
    }
    // Now loop through the faces to populate the indices vector
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]); // Add each index to the list
        }
    }
	Mesh* outputMesh = new Mesh(vertices, indices, lowestX, highestX, lowestY, lowestY, lowestZ, highestZ);
    setMeshToCorrectPos(outputMesh);
    return outputMesh;
}

void ObjectLoader::setMeshToCorrectPos(Mesh* mesh)
{
    float offsetX = -mesh->getLowestX();
    //height offset
    float offsetY = mesh->getLowestY() < 0.0f ? -mesh->getLowestY() : 0.0f;
    float offsetZ = -mesh->getLowestZ();

	float centerX = (static_cast<float>(plateWidth) / 2.0f) -((mesh->getHighestX() - mesh->getLowestX())/2.0f);
	float centerZ = (static_cast<float>(plateDepth) / 2.0f) - ((mesh->getHighestZ() - mesh->getLowestZ()) / 2.0f);
	float height = mesh->getHighestY() - mesh->getLowestY();

    // Apply offsets directly in the loop
    for (auto& vertex : mesh->vertices) {
        glm::vec3 newPosition = vertex.getPosition();
        newPosition.x += (offsetX + centerX);
        //height
        newPosition.y += offsetY;
        //depth
        newPosition.z += (offsetZ + centerZ);
        vertex.setPosition(newPosition);
    }
	//update all the values for lowest and highest x,y,z in mesh
	mesh->setLowestX(mesh->getLowestX() + offsetX + centerX);
	mesh->setHighestX(mesh->getHighestX() + offsetX + centerX);
	mesh->setLowestY(mesh->getLowestY() + offsetY);
	mesh->setHighestY(mesh->getHighestY() + offsetY);
	mesh->setLowestZ(mesh->getLowestZ() + offsetZ + centerZ);
	mesh->setHighestZ(mesh->getHighestZ() + offsetZ + centerZ);

}

