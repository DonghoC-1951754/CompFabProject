#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <clipper2/clipper.h>


class GcodeCreator
{
public:
	GcodeCreator(double maxXDistance, double maxYDistance, int sliceAmount, double filamentDiameter,
		double bedTemp, double nozzleTemp, double nozzleDiameter, float speedMultiplier,
		double layerHeight, bool prime, bool supportToggle, bool retractionToggle,
		const std::vector<Clipper2Lib::PathsD>& erodedSlices,
		const std::vector<std::vector<Clipper2Lib::PathsD>>& shells,
		const std::vector<Clipper2Lib::PathsD>& infill,
		const std::vector<std::vector<Clipper2Lib::PathsD>>& floors,
		const std::vector<std::vector<Clipper2Lib::PathsD>>& roofs,
		const std::vector<Clipper2Lib::PathsD>& erodedSupportPerimeter,
		const std::vector<Clipper2Lib::PathsD>& supportInfill);
	void generateGCode(const std::string& filename);
private:
	// Configuration parameters, moved to private members for easier management
	double maxXDist;
	double maxYDist;
	double layerHeight;
	double filamentDiameter;
	double bedTemp;
	double nozzleTemp;
	double nozzleDiameter;
	double retractionDistance = 7.5;
	float printSpeed;
	int sliceAmount;
	bool prime;
	bool supportToggle;
	bool retractionToggle;

	std::vector<Clipper2Lib::PathsD> erodedSlices;
	std::vector<std::vector<Clipper2Lib::PathsD>> shells;
	std::vector<Clipper2Lib::PathsD> infill;
	std::vector<std::vector<Clipper2Lib::PathsD>> floors;
	std::vector<std::vector<Clipper2Lib::PathsD>> roofs;
	std::vector<Clipper2Lib::PathsD> erodedSupportPerimeter;
	std::vector<Clipper2Lib::PathsD> supportInfill;

	double calculateExtrusionLength(double prevX, double prevY, double currentX, double currentY, double factor = 1.0);
	void writeInitializationGCode(std::ofstream& gcodeFile, const std::string& filename);
	void writeInfillGCode(double& E, std::ofstream& gcodeFile, const Clipper2Lib::PathD& line);

	void writePolygonGCode(bool& firstPolygon, double& E, std::ofstream& gcodeFile, const Clipper2Lib::PathD& polygon);

	void writeSliceGCode(int slice, bool& firstPolygon, double& E, std::ofstream& gcodeFile);

	void retractionStep(double& E, std::ofstream& gcodeFile, Clipper2Lib::PointD nextPoint);

};