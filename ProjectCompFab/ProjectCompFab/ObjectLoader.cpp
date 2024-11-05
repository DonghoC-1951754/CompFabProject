#include "ObjectLoader.h"
#include <iostream>
#include <vector>

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
        Vertex vertex;
        glm::vec3 temp_vector3;

        // Set vertex position
        temp_vector3.x = mesh->mVertices[i].x;
        temp_vector3.y = mesh->mVertices[i].y;
        temp_vector3.z = mesh->mVertices[i].z;
        vertex.setPosition(temp_vector3);

        // Set vertex normal
        temp_vector3.x = mesh->mNormals[i].x;
        temp_vector3.y = mesh->mNormals[i].y;
        temp_vector3.z = mesh->mNormals[i].z;
        vertex.setNormal(temp_vector3);

        // Set texture coordinates if they exist
        if (mesh->mTextureCoords[0]) {
            glm::vec2 temp_vector2;
            temp_vector2.x = mesh->mTextureCoords[0][i].x;
            temp_vector2.y = mesh->mTextureCoords[0][i].y;
            vertex.setTextureCoords(temp_vector2);
        }
        else {
            vertex.setTextureCoords(glm::vec2(0.0f, 0.0f)); // Default texture coordinates
        }

        vertices.push_back(vertex); // Add the vertex to the list
    }

    // Now loop through the faces to populate the indices vector
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]); // Add each index to the list
        }
    }

    return new Mesh(vertices, indices);
}

