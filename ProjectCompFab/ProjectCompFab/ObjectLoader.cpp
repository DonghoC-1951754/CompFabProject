#include "ObjectLoader.h"
#include <iostream>
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
    // First loop through the vertices to populate the vertices vector
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        auto aiVertices = mesh->mVertices[i];
        Vertex vertex;
		vertex.setPosition(glm::vec3(aiVertices.x, aiVertices.z, aiVertices.y));
		vertex.setPosition(glm::vec3(aiVertices.x, aiVertices.z, aiVertices.y));// x = x, y = z: hoogte, z = y: diepte
        //vertex.x = aiVertices.x;
        //vertex.y = aiVertices.z; // Swap Y and Z
        //vertex.z = aiVertices.y; // Swap Y and Z
        vertices.push_back(vertex);
        //vertices = [(breedte, hoogte, diepte),...]
    }
    // Now loop through the faces to populate the indices vector
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]); // Add each index to the list
        }
    }
	Mesh* outputMesh = new Mesh(vertices, indices);
    setMeshToCorrectPos(outputMesh);
    return outputMesh;
}

void ObjectLoader::setMeshToCorrectPos(Mesh* mesh)
{
    std::vector<Vertex> vertices;
	float meshLowestPoint = mesh->getLowestPoint();
    if (meshLowestPoint < 0.0f) {
		float yOffset = -meshLowestPoint;
        for (auto vertex : mesh->vertices) {
			float currentY = vertex.getPosition().y;
            float newY = currentY + yOffset;
			glm::vec3 currentPosition = vertex.getPosition();
            currentPosition.y = newY;
            vertex.setPosition(currentPosition);
            vertices.push_back(vertex);
        }
        mesh->vertices = vertices;
    }
}

