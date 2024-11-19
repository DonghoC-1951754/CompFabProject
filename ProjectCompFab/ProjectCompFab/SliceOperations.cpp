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

std::vector<Clipper2Lib::PathsD> SliceOperations::addShells(const std::vector<Clipper2Lib::PathsD> slices, int shellAmount, double nozzleDiameter)
{
    mostInnerShells.clear();
    std::vector<Clipper2Lib::PathsD> shelledSlices;
    double stepSize = -nozzleDiameter / 2;
    for (auto slice : slices) {
        Clipper2Lib::PathsD shells;
        Clipper2Lib::PathsD shell = slice;
        for (int i = 0; i < shellAmount; i++) {
            shell = Clipper2Lib::InflatePaths(shell, stepSize, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon, 2);
            for (auto path : shell) {
                shells.push_back(path);
            }
            if (i == shellAmount - 1) {
                mostInnerShells.push_back(shell);
            }
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

    for (auto slice : slices) {
        Clipper2Lib::ClipperD clipper;
        clipper.AddClip(slice);
        clipper.AddOpenSubject(infillGrid);
        Clipper2Lib::PathsD infill;
        Clipper2Lib::PathsD openInfill;
        clipper.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::EvenOdd, infill, openInfill);
        infilledSlices.push_back(openInfill);
    }
    return infilledSlices;
}


Clipper2Lib::PathsD SliceOperations::generateInfillGrid(double buildPlateWidth, double buildPlateDepth, double infillDensity)
{
    Clipper2Lib::PathsD grid;
    for (double x = 0; x <= buildPlateWidth; x += infillDensity) {
        Clipper2Lib::PathD verticalLine;
        verticalLine.push_back(Clipper2Lib::PointD(x, 0.0));
        verticalLine.push_back(Clipper2Lib::PointD(x, buildPlateDepth));
        grid.push_back(verticalLine);
    }
    for (double y = 0; y <= buildPlateDepth; y += infillDensity) {
        Clipper2Lib::PathD horizontalLine;
        horizontalLine.push_back(Clipper2Lib::PointD(0.0, y));
        horizontalLine.push_back(Clipper2Lib::PointD(buildPlateWidth, y));
        grid.push_back(horizontalLine);
    }
    return grid;
}