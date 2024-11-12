#pragma once
#include <vector>
#include "Mesh.h"
class SlicerPlane
{
public:
	SlicerPlane() {};
	std::vector<float> getVertices() { return vertices; };
	std::vector<int> getIndices() { return indices; };
	std::vector<std::vector<glm::vec3>> slice(const Mesh* mesh, double slicerHeight);
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
	double epsilon = 0.00000001;
	std::vector<std::vector<glm::vec3>> orderedLineSegments;
	std::vector<std::vector<glm::vec3>> lineSegments;
	std::vector<std::vector<glm::vec3>> pointSegments;
	bool nextIsPointSegment(std::vector<glm::vec3> lineSegment);
	void calcLineSegments(std::vector<Vertex> triangle, double slicerHeight);
	void calcIntersectionPoint(Vertex a, Vertex b, double slicerHeight);
};

