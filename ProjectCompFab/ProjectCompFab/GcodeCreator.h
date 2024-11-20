#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <clipper2/clipper.h>


class GcodeCreator
{
public:
	GcodeCreator();
	void generateGCode(const int sliceAmount, const std::vector<Clipper2Lib::PathsD> erodedSlices, const std::vector<Clipper2Lib::PathsD> shells, const std::vector<Clipper2Lib::PathsD> infill, 
		const std::string& filename, double layerHeight = 0.2, double filamentDiameter = 1.75, double bedTemp = 60.0, double nozzleTemp = 200.0, double nozzleDiameter = 0.4, bool prime = true);
private:
	double calculateExtrusionLength(double prevX, double prevY, double currentX, double currentY, double filamentDiameter, double layerHeight, double nozzleDiameter);
};