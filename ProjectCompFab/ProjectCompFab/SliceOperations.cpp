#include "SliceOperations.h"


std::vector<Clipper2Lib::PathsD> SliceOperations::erodeSlicesForGCode(const std::vector<Clipper2Lib::PathsD> slices, double nozzleDiameter)
{
    std::vector<Clipper2Lib::PathsD> erodedSlices;
    double offsetValue = -nozzleDiameter / 2;
    for (auto slice : slices) {
        Clipper2Lib::PathsD erodedPaths = Clipper2Lib::InflatePaths(slice, offsetValue, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon, 2);
        erodedSlices.push_back(erodedPaths);
    }

    return erodedSlices;
}

std::vector<std::vector<Clipper2Lib::PathsD>> SliceOperations::addShells(const std::vector<Clipper2Lib::PathsD> slices, int shellAmount, double nozzleDiameter)
{
    mostInnerShells.clear();
    std::vector<std::vector<Clipper2Lib::PathsD>> shelledSlices;
    double stepSize = -nozzleDiameter;
    for (auto slice : slices) {
        std::vector<Clipper2Lib::PathsD> shells;
        Clipper2Lib::PathsD shell = slice;
        for (int i = 0; i < shellAmount; i++) {
            Clipper2Lib::PathsD currentShell;
            shell = Clipper2Lib::InflatePaths(shell, stepSize, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon, 2);
            for (auto path : shell) {
                currentShell.push_back(path);
            }
            if (i == shellAmount - 1) {
                mostInnerShells.push_back(shell);
            }
            shells.push_back(currentShell);
        }
        shelledSlices.push_back(shells);
    }
    return shelledSlices;
}

std::vector<Clipper2Lib::PathsD> SliceOperations::generateInfill(const std::vector<Clipper2Lib::PathsD> innerShells, const std::vector<Clipper2Lib::PathsD> erodedSlices, double infillDensity)
{
    std::vector<Clipper2Lib::PathsD> infilledSlices;
    Clipper2Lib::PathsD infillGrid = generateInfillGrid(200, 200, infillDensity);
    std::vector<Clipper2Lib::PathsD> slices;

    if (innerShells.size() == 0) slices = erodedSlices;
    else slices = innerShells;

    int i = 0;
    for (auto slice : slices) {
        Clipper2Lib::ClipperD clipper;
        clipper.AddClip(slice);
        clipper.AddOpenSubject(infillGrid);
        Clipper2Lib::PathsD infill;
        Clipper2Lib::PathsD openInfill;
        clipper.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::EvenOdd, infill, openInfill);
		clipper.Clear();
        clipper.AddOpenSubject(openInfill);
		clipper.AddClip(allFloorRegions[i]);
        clipper.AddClip(allRoofRegions[i]);
		clipper.Execute(Clipper2Lib::ClipType::Difference, Clipper2Lib::FillRule::EvenOdd, infill, openInfill);
        clipper.Clear();
        infilledSlices.push_back(openInfill);
        ++i;
    }
    return infilledSlices;
}

std::vector<std::vector<Clipper2Lib::PathsD>> SliceOperations::generateRoofsAndFloorsInfill(std::vector<Clipper2Lib::PathsD> perimeter, int baseFloorAmount, bool isFloor, double nozzleDiameter)
{
    std::vector<Clipper2Lib::PathsD> allRegions;
    if (isFloor) {
        allFloorRegions = calcRoofsAndFloorRegions(baseFloorAmount, perimeter, true);
		allRegions = allFloorRegions;
    }
    else {
		allRoofRegions = calcRoofsAndFloorRegions(baseFloorAmount, perimeter, false);
		std::reverse(allRoofRegions.begin(), allRoofRegions.end());
		allRegions = allRoofRegions;
    }
    std::vector<std::vector<Clipper2Lib::PathsD>> allRingInfills;
    for (auto regions : allRegions) {
        std::vector<Clipper2Lib::PathsD> ringInfillSingleSlice;
        auto singleRing = Clipper2Lib::InflatePaths(regions, -nozzleDiameter, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon, 2);
		//ringInfillSingleSlice.push_back(singleRing);
        while (!singleRing.empty()) {
            ringInfillSingleSlice.push_back(singleRing);
            singleRing = Clipper2Lib::InflatePaths(singleRing, -nozzleDiameter, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon, 2);
        }
		allRingInfills.push_back(ringInfillSingleSlice);
    }
    return allRingInfills;
}

std::vector<Clipper2Lib::PathsD> SliceOperations::generateErodedSupportPerimeter(const std::vector<Clipper2Lib::PathsD> slices, double nozzleDiameter, double layerHeight)
{
	auto supportRegions = calcSupportRegions(slices, nozzleDiameter, layerHeight);
    // Erode ?
    return supportRegions;
}

//std::vector<Clipper2Lib::PathsD> SliceOperations::generateSupportInfill(const std::vector<Clipper2Lib::PathsD> supportPerimeters, double infillDensity)
//{
//    Clipper2Lib::PathsD infillGrid = generateInfillGrid(200, 200, infillDensity);
//    return std::vector<Clipper2Lib::PathsD>();
//}


Clipper2Lib::PathsD SliceOperations::generateInfillGrid(double buildPlateWidth, double buildPlateDepth, double infillDensity)
{
    Clipper2Lib::PathsD grid;
	bool traversed = false;
    for (double x = 0; x <= buildPlateWidth; x += infillDensity) {
        Clipper2Lib::PathD verticalLine;
        if (traversed) {
            verticalLine.push_back(Clipper2Lib::PointD(x, 0.0));
            verticalLine.push_back(Clipper2Lib::PointD(x, buildPlateDepth));
            traversed = false;
        }
        else {
			verticalLine.push_back(Clipper2Lib::PointD(x, buildPlateDepth));
			verticalLine.push_back(Clipper2Lib::PointD(x, 0.0));
			traversed = true;
        }
        grid.push_back(verticalLine);
    }
    for (double y = 0; y <= buildPlateDepth; y += infillDensity) {
        Clipper2Lib::PathD horizontalLine;
        if (traversed) {
            horizontalLine.push_back(Clipper2Lib::PointD(0.0, y));
            horizontalLine.push_back(Clipper2Lib::PointD(buildPlateWidth, y));
			traversed = false;
        }
        else {
			horizontalLine.push_back(Clipper2Lib::PointD(buildPlateWidth, y));
			horizontalLine.push_back(Clipper2Lib::PointD(0.0, y));
			traversed = true;
        }
        
        grid.push_back(horizontalLine);
    }
    return grid;
}

std::vector<Clipper2Lib::PathsD> SliceOperations::calcRoofsAndFloorRegions(int baseFloorAmount, std::vector<Clipper2Lib::PathsD> perimeter, bool isFloor)
{
    std::vector<Clipper2Lib::PathsD> mostInnerShell;
	if (mostInnerShells.size() == 0) mostInnerShell = perimeter;
	else mostInnerShell = mostInnerShells;

    if (!isFloor) {
		std::reverse(mostInnerShell.begin(), mostInnerShell.end());
    }

	std::vector<Clipper2Lib::PathsD> regions;
    for (int j = 0; j < baseFloorAmount; ++j) {
        regions.push_back(mostInnerShell[j]);
    }
    for (int i = baseFloorAmount; i < mostInnerShell.size(); ++i) {
        auto currentLayer = mostInnerShell[i];
		std::vector<Clipper2Lib::PathsD> prevLayers;
        for (int k = 1; k <= baseFloorAmount; ++k) {
			prevLayers.push_back(mostInnerShell[i - k]);
        }

        if (baseFloorAmount == 0) {
			regions.push_back(Clipper2Lib::PathsD());
        }
        else if (baseFloorAmount == 1) {
            auto diff = Difference(currentLayer, mostInnerShell[i - 1], Clipper2Lib::FillRule::EvenOdd);
			regions.push_back(diff);
        }
        else {
            Clipper2Lib::PathsD intersection;
            for (int l = 0; l < prevLayers.size() - 1; ++l) {
                intersection = Intersect(prevLayers[l], prevLayers[l + 1], Clipper2Lib::FillRule::EvenOdd);
            }
            auto clippedRegion = Difference(currentLayer, intersection, Clipper2Lib::FillRule::EvenOdd);
            regions.push_back(clippedRegion);
        }
        
    }

	return regions;
}

std::vector<Clipper2Lib::PathsD> SliceOperations::calcSupportRegions(const std::vector<Clipper2Lib::PathsD> slices, double nozzleDiameter, double layerHeight)
{
    // Self-supporting area
    double a = std::min(nozzleDiameter/2, layerHeight);
    std::vector<Clipper2Lib::PathsD> supportRegions;
	supportRegions.push_back(Clipper2Lib::PathsD());

	std::vector<Clipper2Lib::PathsD> supportWithoutSelfSupport;
	supportWithoutSelfSupport.push_back(Clipper2Lib::PathsD());

    Clipper2Lib::ClipperD clipper;

    for (int i = slices.size() - 2; i >= 0; --i) {
        clipper.AddSubject(slices[i + 1]);
        clipper.AddClip(supportWithoutSelfSupport.back());
        Clipper2Lib::PathsD unionPerimeterSupport;
		clipper.Execute(Clipper2Lib::ClipType::Union, Clipper2Lib::FillRule::EvenOdd, unionPerimeterSupport);
		supportWithoutSelfSupport.push_back(unionPerimeterSupport);
		clipper.Clear();
    }
    int k = 0;
    for (int j = slices.size() - 2; j >= 0; --j) {
        clipper.AddSubject(slices[j + 1]);
        clipper.AddClip(supportWithoutSelfSupport[k]);
        Clipper2Lib::PathsD unionPerimeterSupport;
        clipper.Execute(Clipper2Lib::ClipType::Union, Clipper2Lib::FillRule::EvenOdd, unionPerimeterSupport);
		clipper.Clear();

        auto dilatedCurrentPerimeter = Clipper2Lib::InflatePaths(slices[j], a, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon);

        clipper.AddSubject(unionPerimeterSupport);
		clipper.AddClip(dilatedCurrentPerimeter);
		Clipper2Lib::PathsD support;
		clipper.Execute(Clipper2Lib::ClipType::Difference, Clipper2Lib::FillRule::EvenOdd, support);
		clipper.Clear();
		supportRegions.push_back(support);
        ++k;
    }
	std::reverse(supportRegions.begin(), supportRegions.end());
    return supportRegions;
}
