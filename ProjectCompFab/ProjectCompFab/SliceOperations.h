#pragma once
#include <vector>
#include "clipper2/clipper.h"

class SliceOperations
{
private:
	std::vector<Clipper2Lib::PathsD> mostInnerShells;
	Clipper2Lib::PathsD generateInfillGrid(double buildPlateWidth, double buildPlateDepth, double infillDensity);
public:
	std::vector<Clipper2Lib::PathsD> erodeSlicesForGCode(const std::vector<Clipper2Lib::PathsD> slices, double nozzleDiameter);
	std::vector<Clipper2Lib::PathsD> addShells(const std::vector<Clipper2Lib::PathsD> slices, int shellAmount, double nozzleDiameter);
	std::vector<Clipper2Lib::PathsD> generateInfill(const std::vector<Clipper2Lib::PathsD> innerShells, const std::vector<Clipper2Lib::PathsD> erodedSlices);
	std::vector<Clipper2Lib::PathsD> getMostInnerShells() { return mostInnerShells; }
};

