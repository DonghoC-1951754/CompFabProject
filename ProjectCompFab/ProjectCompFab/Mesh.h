#pragma once
#include <vector>
#include "Vertex.h"
#include "Texture.h"

class Mesh{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;


    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;
    }
};