#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <clipper2/clipper.h>


class GcodeCreator
{
public:
	GcodeCreator();
	void generateGCode(const double maxXDist, const double maxYDist, const int sliceAmount, const std::vector<Clipper2Lib::PathsD> erodedSlices,
		const std::vector<std::vector<Clipper2Lib::PathsD>> shells, const std::vector<Clipper2Lib::PathsD> infill, const std::vector<std::vector<Clipper2Lib::PathsD>> floors,
		const std::vector<std::vector<Clipper2Lib::PathsD>> roofs, const std::vector<Clipper2Lib::PathsD> erodedSupportPerimeter,
		const std::vector<Clipper2Lib::PathsD> supportInfill,
		const std::string& filename, double layerHeight, double filamentDiameter, double bedTemp, double nozzleTemp, double nozzleDiameter, bool prime);
private:
	double calculateExtrusionLength(double prevX, double prevY, double currentX, double currentY, double filamentDiameter, double layerHeight, double nozzleDiameter, double factor = 1.0);
	void writeInitializationGCode(std::ofstream& gcodeFile, double bedTemp, double nozzleTemp, double layerHeight, bool prime);
	void writePolygonGCode(bool& firstPolygon, bool& firstPoint, double& E, double retractionDistance, std::ofstream& gcodeFile,
		const Clipper2Lib::PathD& polygon, double filamentDiameter, double layerHeight, double nozzleDiameter);
	void writeSliceGCode(int slice, bool& firstPolygon, bool& firstPoint, double& E, double retractionDistance, std::ofstream& gcodeFile,
		double filamentDiameter, double layerHeight, double nozzleDiameter, const std::vector<Clipper2Lib::PathsD>& erodedSlices,
		const std::vector<std::vector<Clipper2Lib::PathsD>>& shells, const std::vector<Clipper2Lib::PathsD>& infill, const std::vector<std::vector<Clipper2Lib::PathsD>> floors,
		const std::vector<std::vector<Clipper2Lib::PathsD>> roofs, const std::vector<Clipper2Lib::PathsD> erodedSupportPerimeter, const std::vector<Clipper2Lib::PathsD> supportInfill);
	
	double maxXDistance;
	double maxYDistance;
	double printSpeed = 2000.0;
};