#pragma once
#include <vector>
#include "clipper2/clipper.h"

class SliceOperations
{
private:
	std::vector<Clipper2Lib::PathsD> mostInnerShells;
	std::vector<Clipper2Lib::PathsD> allFloorRegions;
	std::vector<Clipper2Lib::PathsD> allRoofRegions;
	Clipper2Lib::PathsD generateInfillGrid(double buildPlateWidth, double buildPlateDepth, double infillDensity);
	std::vector<Clipper2Lib::PathsD> calcRoofsAndFloorRegions(int baseFloorAmount, std::vector<Clipper2Lib::PathsD> perimeter, bool isFloor);
public:
	std::vector<Clipper2Lib::PathsD> erodeSlicesForGCode(const std::vector<Clipper2Lib::PathsD> slices, double nozzleDiameter);
	std::vector<std::vector<Clipper2Lib::PathsD>> addShells(const std::vector<Clipper2Lib::PathsD> slices, int shellAmount, double nozzleDiameter);
	std::vector<Clipper2Lib::PathsD> generateInfill(const std::vector<Clipper2Lib::PathsD> innerShells, const std::vector<Clipper2Lib::PathsD> erodedSlices, double infillDensity);
	std::vector<Clipper2Lib::PathsD> getMostInnerShells() { return mostInnerShells; }
	std::vector<std::vector<Clipper2Lib::PathsD>> generateRoofsAndFloorsInfill(std::vector<Clipper2Lib::PathsD> perimeter, int baseFloorAmount, bool isFloor);
};

