#pragma once
#include <vector>
#include "Mesh.h"
class SlicerPlane
{
public:
	SlicerPlane() {};
	std::vector<float> getVertices() { return vertices; };
	std::vector<int> getIndices() { return indices; };
	void slice(const Mesh* mesh, float slicerHeight);
	std::vector<std::vector<glm::vec3>> getOrderedLineSegments();

private:
	float width = 40.0f;
    std::vector<float> vertices{
        -width, 0.0f, -width,
		 width, 0.0f, -width,
		 width, 0.0f,  width,
        -width, 0.0f,  width,
    };
	std::vector<int> indices{
		0, 1, 2,
		2, 3, 0
	};

	std::vector<std::vector<glm::vec3>> lineSegments;

	void calcLineSegments(std::vector<Vertex> triangle, float slicerHeight);
	void calcIntersectionPoint(Vertex a, Vertex b, float slicerHeight);
};

