#pragma once
#include <vector>
#include "Vertex.h"
#include "Texture.h"

class Mesh{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;


    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, float lowestX, float lowestY, float lowestZ, float highestZ) {
        this->vertices = vertices;
        this->indices = indices;
		this->lowestX = lowestX;
		this->lowestY = lowestY;
		this->lowestZ = lowestZ;
		this->highestZ = highestZ;
    }

	float getLowestX() {return lowestX;}
	float getLowestY() {return lowestY;}
	float getLowestZ() {return lowestZ;}
	float getHighestZ() {return highestZ;}

	void setLowestX(float x) {lowestX = x;}
	void setLowestY(float y) {lowestY = y;}
	void setLowestZ(float z) {lowestZ = z;}
	void setHighestZ(float z) {highestZ = z;}

private:
    float lowestX;
    float lowestY;
	float lowestZ;
	float highestZ;

};