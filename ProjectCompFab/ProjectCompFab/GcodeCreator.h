#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <clipper2/clipper.h>


class GcodeCreator
{
	using Polygon = std::vector<std::vector<glm::dvec3>>;
public:
	void generateGCode(const std::vector<Polygon> slicePolygons, const std::string& filename, double layerHeight = 0.2, double bedTemp = 60.0,
	double nozzleTemp = 200.0, double nozzleDiameter = 0.4, bool prime = true);
};