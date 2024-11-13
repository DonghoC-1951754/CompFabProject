#pragma once
#include <vector>
#include "Mesh.h"
#include "clipper2/clipper.h"

class SlicerPlane
{

public:
	SlicerPlane() {};
	std::vector<float> getVertices() { return vertices; };
	std::vector<int> getIndices() { return indices; };
	std::vector< std::vector<std::vector<glm::dvec3>>> slice(const Mesh* mesh, double slicerHeight);
	void setContours(std::vector< std::vector<std::vector<glm::dvec3>>> polygons);

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
	double epsilon = 0.000000001;
	std::vector<std::vector<std::vector<glm::dvec3>>> polygonsOfOrderedLineSegments;
	std::vector<std::vector<glm::dvec3>> lineSegments;
	void calcLineSegments(std::vector<Vertex> triangle, double slicerHeight);
	void fillPolygonWithOrderedSegments();
	std::vector<std::vector<glm::dvec3>> getSingleOrderedPolygon(std::vector<std::vector<glm::dvec3>>& remainingUnorderedLineSegments);
	std::vector<glm::dvec3> getFlattenedPolygon(std::vector<std::vector<glm::dvec3>> singlePolygon);

	Clipper2Lib::PathsD contours;
};

