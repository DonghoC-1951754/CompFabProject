#include "GcodeCreator.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <qmath.h>

Clipper2Lib::PathsD GcodeCreator::generateInfillGrid(double buildPlateWidth, double buildPlateDepth, double infillDensity)
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

void GcodeCreator::generateGCode(const std::vector<Polygon> slicePolygons, const std::string& filename, double layerHeight, double bedTemp, double nozzleTemp,
    double nozzleDiameter, bool prime) {
    std::ofstream gcodeFile(filename + "\.gcode");

    if (!gcodeFile.is_open()) {
        std::cerr << "Failed to open file for writing.\n";
        return;
    }

    // Write initialization G-code
    gcodeFile << "%";
    gcodeFile << "M140 S" + std::to_string(bedTemp) + "\n"; // Set bed temp
    gcodeFile << "M104 S" + std::to_string(nozzleTemp) + "\n"; // Set nozzle temp
    gcodeFile << "M190 S" + std::to_string(bedTemp) + "\n"; // Wait for bed temp
    gcodeFile << "M109 S" + std::to_string(nozzleTemp) + "\n"; // Wait for nozzle temp
    gcodeFile << "M82\n"; // Absolute extrusion mode
    gcodeFile << "G28\n";       // Home all axes
    gcodeFile << "G92 E0\n";    // Reset extruder
    
    // Prime the nozzle
    if (prime){
        gcodeFile << "G1 Z2.0 F3000\n";
        gcodeFile << "G1 X0.1 Y20 Z" + std::to_string(layerHeight) + " F5000\n";
        gcodeFile << "G1 X0.1 Y150 Z" + std::to_string(layerHeight) + " F1500 E15\n";
        gcodeFile << "G1 X0.4 Y150 Z" + std::to_string(layerHeight) + " F5000\n";
        gcodeFile << "G1 X0.4 Y20 Z" + std::to_string(layerHeight) + " F1500 E30\n";
        gcodeFile << "G1 Z" + std::to_string(layerHeight) + " F3000\n";
        gcodeFile << "M107\n";
    }
    

    // Actual printing
    gcodeFile << "G1 Z" + std::to_string(layerHeight) + " F1200\n"; // Set feed rate
    // OUR Y BECOMES THE Z IN THE G-CODE, MEANING WE USE Y FOR HEIGHT WHILE THE G-CODE USES Z. AND VICE VERSA
    double E = 0.0;
	double currentLayer = 0;
    for (const auto& polygon : slicePolygons) {
		const glm::vec3 firstEdge = polygon[0][0];
		currentLayer += 1;
        gcodeFile << "G1 X" << firstEdge.x << " Y" << firstEdge.z << " Z" << currentLayer*layerHeight << "\n";
        for (const auto& vertex : polygon) {
			const glm::vec3 startEdge = vertex[0];
			const glm::vec3 endEdge = vertex[1];

			double distance = glm::distance(startEdge,endEdge);
            //extrustionLength = (Layer height x Nozzle diameter x L)/Filament area 

			double extrusionLength = (distance * layerHeight * nozzleDiameter)/ M_PI * pow(nozzleDiameter / 2, 2);

            E += distance*0.55; // Adjust extrusion multiplier as needed

            gcodeFile << std::fixed << std::setprecision(4);
            gcodeFile << "G1 X" << endEdge.x << " Y" << endEdge.z << " E" << E << "\n";
        }
    }

    // Finish G-code
    gcodeFile << "G1 E-2 F2400\n"; // Retract filament
    gcodeFile << "G1 Z10 F3000\n"; // Move nozzle up
    gcodeFile << "M104 S0\n";      // Turn off hotend
    gcodeFile << "M140 S0\n";      // Turn off bed
    gcodeFile << "M84\n";          // Disable motors
    gcodeFile << "; End of G-code\n";

    gcodeFile.close();
    std::cout << "G-code written to " << filename << "\n";
}

std::vector<Clipper2Lib::PathsD> GcodeCreator::erodeSlicesForGCode(const std::vector<Clipper2Lib::PathsD> slices, double nozzleDiameter)
{
	std::vector<Clipper2Lib::PathsD> erodedSlices;
    double offsetValue = -nozzleDiameter / 2;
    for (auto slice : slices) {
        Clipper2Lib::PathsD erodedPaths = Clipper2Lib::InflatePaths(slice, offsetValue, Clipper2Lib::JoinType::Square, Clipper2Lib::EndType::Polygon, 2);
		erodedSlices.push_back(erodedPaths);
    }

    return erodedSlices;
}

std::vector<Clipper2Lib::PathsD> GcodeCreator::addShells(const std::vector<Clipper2Lib::PathsD> slices, int shellAmount, double nozzleDiameter)
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

std::vector<Clipper2Lib::PathsD> GcodeCreator::generateInfill(const std::vector<Clipper2Lib::PathsD> innerShells, const std::vector<Clipper2Lib::PathsD> erodedSlices)
{
	std::vector<Clipper2Lib::PathsD> infilledSlices;
    Clipper2Lib::PathsD infillGrid = generateInfillGrid(200, 200, 1);
    std::vector<Clipper2Lib::PathsD> slices;
    if (innerShells.size() == 0) {
		slices = erodedSlices;
    }
    else {
		slices = innerShells;
    }

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
