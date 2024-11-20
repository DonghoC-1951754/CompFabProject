#pragma once
#include <vector>
#include "Vertex.h"
#include "Texture.h"

class Mesh{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;


    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, float lowestX, float highestX, float lowestY, float highestY, float lowestZ, float highestZ) {
        this->vertices = vertices;
        this->indices = indices;
		this->lowestX = lowestX;
		this->highestX = highestX;
		this->lowestY = lowestY;
		this->highestY = highestY;
		this->lowestZ = lowestZ;
		this->highestZ = highestZ;
    }

	float getLowestX() {return lowestX;}
	float getHighestX() { return highestX; }
	float getLowestY() {return lowestY;}
	float getHighestY() { return highestY; }
	float getLowestZ() {return lowestZ;}
	float getHighestZ() {return highestZ;}

	void setLowestX(float x) {lowestX = x;}
	void setHighestX(float x) { highestX = x; }
	void setLowestY(float y) {lowestY = y;}
	void setHighestY(float y) { highestY = y; }
	void setLowestZ(float z) {lowestZ = z;}
	void setHighestZ(float z) {highestZ = z;}

private:
    float lowestX;
	float highestX;
    float lowestY;
	float highestY;
	float lowestZ;
	float highestZ;

};