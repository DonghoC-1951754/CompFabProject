#include "Mesh.h"

float Mesh::getLowestPoint()
{
	float lowest = vertices[0].getPosition().y;
	for (const auto& vertex : vertices) {
		if (vertex.getPosition().y < lowest) {
			lowest = vertex.getPosition().y;
		}
	}
	return lowest;
}

float Mesh::getHighestPoint()
{
	float highest = vertices[0].getPosition().y;
	for (const auto& vertex : vertices) {
		if (vertex.getPosition().y > highest) {
			highest = vertex.getPosition().y;
		}
	}
	return highest;
}
