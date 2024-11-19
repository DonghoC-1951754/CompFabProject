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
	Clipper2Lib::PathsD compilePolygons();
	void setLayerHeight(double layerHeight) { this->layerHeight = layerHeight; };
	double getLayerHeight() { return layerHeight; };
	void setWidth(float width) { this->width = width; };
	void setDepth(float depth) { this->width = depth; };
	void setStandardHeight(float standardHeight) { this->standardHeight = standardHeight; };

private:
	float width = 180.0f;
	float depth = 180.0f;
	float standardHeight = 0.2f;
    std::vector<float> vertices{
		0.0f, standardHeight, 0.0f,       // Bottom-left corner (0, 0)
		width + 10.0f, standardHeight, 0.0f,  // Bottom-right corner (18, 0)
		width + 10.0f, standardHeight, -(depth + 10.0f),  // Top-right corner (18, -18)
		0.0f, standardHeight, -(depth + 10.0f)   // Top-left corner (0, -18)
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

	double layerHeight = 0.2;

	Clipper2Lib::PathsD contours;
};

