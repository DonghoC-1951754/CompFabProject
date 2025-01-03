#include "GcodeCreator.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <qmath.h>
#include <QDebug>
#include <filesystem>


GcodeCreator::GcodeCreator(double maxXDistance, double maxYDistance, float bedWidth, float bedDepth, int sliceAmount, double filamentDiameter,
	double bedTemp, double nozzleTemp, double nozzleDiameter, float speedMultiplier,
    double layerHeight, bool prime, bool supportToggle, bool retractionToggle, bool speedToggle,
    const std::vector<Clipper2Lib::PathsD>& erodedSlices,
    const std::vector<std::vector<Clipper2Lib::PathsD>>& shells,
    const std::vector<Clipper2Lib::PathsD>& infill,
    const std::vector<std::vector<Clipper2Lib::PathsD>>& floors,
    const std::vector<std::vector<Clipper2Lib::PathsD>>& roofs,
    const std::vector<Clipper2Lib::PathsD>& erodedSupportPerimeter,
    const std::vector<Clipper2Lib::PathsD>& supportInfill)
    : maxXDist(maxXDistance),
    maxYDist(maxYDistance),
	bedWidth(bedWidth),
	bedDepth(bedDepth),
    sliceAmount(sliceAmount),
    filamentDiameter(filamentDiameter),
    bedTemp(bedTemp),
    nozzleTemp(nozzleTemp),
    nozzleDiameter(nozzleDiameter),
    layerHeight(layerHeight),
    prime(prime),
    supportToggle(supportToggle),
	retractionToggle(retractionToggle),
	speedToggle(speedToggle),
    erodedSlices(erodedSlices),  // Initialize with passed list
    shells(shells),              // Initialize with passed 2D vector
    infill(infill),              // Initialize with passed list
    floors(floors),              // Initialize with passed 2D vector
    roofs(roofs),                // Initialize with passed 2D vector
    erodedSupportPerimeter(erodedSupportPerimeter), // Initialize with passed list
    supportInfill(supportInfill), // Initialize with passed list
    printSpeed(speedMultiplier * 3000.0),
	minSpeedMultiplier(0.5),
	speedCalcCutoff(5.0),
	retractionDistance(7.5)
{
}


void GcodeCreator::generateGCode(const std::string& filename) {
    const std::string directory = "./gcode";
    if (!std::filesystem::exists(directory)) {
        if (!std::filesystem::create_directory(directory)) {
            std::cerr << "Failed to create the /gcode directory.\n";
            return;
        }
    }

    // Open the file for writing
    std::ofstream gcodeFile(directory + "/" + filename + ".gcode");
    gcodeFile << std::fixed << std::setprecision(8);

    if (!gcodeFile.is_open()) {
        std::cerr << "Failed to open file for writing.\n";
        return;
    }

    writeInitializationGCode(gcodeFile, filename);

    double E = 0.0;
    bool firstPolygon = true;

    for (int slice = 0; slice < sliceAmount; slice++) {
        writeSliceGCode(slice, firstPolygon, E, gcodeFile);
    }

    E -= retractionDistance;
    // Finish G-code
    gcodeFile << "G1 E" << E << "F2400\n"; // Retract filament
    gcodeFile << "G1 Z" << (layerHeight * sliceAmount) + 1 << " F3000\n"; // Move nozzle up
    gcodeFile << "G0 X0.5 Y" << bedDepth-0.5 << " E" << E << "F3000\n";
    gcodeFile << "M106 S0\n";          // Turn off fan
    gcodeFile << "M104 S0\n";      // Turn off hotend
    gcodeFile << "M140 S0\n";      // Turn off bed
    gcodeFile << "M84\n";          // Disable motors
    gcodeFile << "; End of G-code\n";

    gcodeFile.close();
}

void GcodeCreator::writeSliceGCode(int slice, bool& firstPolygon, double& E, std::ofstream& gcodeFile) {
    gcodeFile << "; Slice " << slice << "\n";
    gcodeFile << "G0 Z" << (layerHeight * (slice + 1)) << "\n"; // Move to the current layer

    gcodeFile << "; Perimeter" << "\n";
    //ERODED SLICES
    for (const auto& polygon : erodedSlices[slice]) {
        writePolygonGCode(firstPolygon, E, gcodeFile, polygon);
    }

    gcodeFile << "; Shells" << "\n";
    //SHELLS
    for (const auto& setOfShells : shells[slice]) {
        for (const auto& shell : setOfShells) {
            writePolygonGCode(firstPolygon, E, gcodeFile, shell);
        }
    }

    gcodeFile << "; Floors" << "\n";
    //FLOORS
    for (const auto& setOfFloors : floors[slice]) {
        for (const auto& floor : setOfFloors) {
            writePolygonGCode(firstPolygon, E, gcodeFile, floor);
        }
    }

    gcodeFile << "; Roofs" << "\n";
    //ROOFS
    for (const auto& setOfRoofs : roofs[slice]) {
        for (const auto& roof : setOfRoofs) {
            writePolygonGCode(firstPolygon, E, gcodeFile, roof);
        }
    }

    gcodeFile << "; Infill" << "\n";
    //INFILL
    for (const auto& line : infill[slice]) {
		writeInfillGCode(E, gcodeFile, line);
    }

    if (supportToggle) {
        gcodeFile << "; Support Perimeter" << "\n";
        //SUPPORT PERIMETER
        if (!erodedSupportPerimeter[slice].empty()) {
            for (const auto& supportPolygon : erodedSupportPerimeter[slice]) {
                writePolygonGCode(firstPolygon, E, gcodeFile, supportPolygon);
            }
        }

        gcodeFile << "; Support Infill" << "\n";
        //SUPPORT INFILL
        if (!supportInfill[slice].empty()) {
            for (const auto& supportLine : supportInfill[slice]) {
				writeInfillGCode(E, gcodeFile, supportLine);
            }
        }
    }


}

void GcodeCreator::writeInfillGCode(double& E, std::ofstream& gcodeFile, const Clipper2Lib::PathD& line) {
    if (retractionToggle) {
        E -= retractionDistance;
        gcodeFile << "G1 E" << E << " F6000\n";
    }
    gcodeFile << "G0 X" << line[0].x << " Y" << line[0].y << "\n";

    double segmentLength = calculateSegmentLength(line[0].x, line[0].y, line[1].x, line[1].y);  // Calculate segment length
    double adjustedSpeed = printSpeed;  // Default to printSpeed

    if (speedToggle and segmentLength <= speedCalcCutoff) {
        adjustedSpeed = calculateAdjustedSpeed(segmentLength);  // Adjust speed based on segment length
    }

    if (retractionToggle) {
        E += retractionDistance;
        gcodeFile << "G1 E" << E << " F" << adjustedSpeed << "\n"; // Restore filament dynamically
    }
	else {
		gcodeFile << "G1 F" << adjustedSpeed << "\n"; // Set feed rate
	}
    E += calculateExtrusionLength(line[0].x, line[0].y, line[1].x, line[1].y);
    gcodeFile << "G1 X" << line[1].x << " Y" << line[1].y << " E" << E << "\n";
}

void GcodeCreator::writePolygonGCode(bool& firstPolygon, double& E, std::ofstream& gcodeFile, const Clipper2Lib::PathD& polygon) {
    if (!firstPolygon) {
        if (retractionToggle) {
            retractionStep(E, gcodeFile, polygon[0]);
        }
    }
    firstPolygon = false; 

    double prevX = polygon[0].x, prevY = polygon[0].y;
    bool firstPoint = true; // To check if it's the first point in the polygon

    for (const auto& point : polygon) {
        double adjustedSpeed = printSpeed;  // Default to printSpeed
        if (!firstPoint) {
            E += calculateExtrusionLength(prevX, prevY, point.x, point.y);
            double segmentLength = calculateSegmentLength(prevX, prevY, point.x, point.y);
            if (speedToggle and segmentLength <= speedCalcCutoff) {
                adjustedSpeed = calculateAdjustedSpeed(segmentLength);  // Adjust speed based on segment length
            }
        }
        gcodeFile << "G1 X" << point.x << " Y" << point.y << " E" << E << " F" << adjustedSpeed <<"\n";
        // Update the previous point
        prevX = point.x;
        prevY = point.y;
        firstPoint = false;
    }
    double finalSegmentLength = calculateSegmentLength(prevX, prevY, polygon[0].x, polygon[0].y); // Last segment
    double finalAdjustedSpeed = printSpeed;  // Default to printSpeed

    if (speedToggle and finalSegmentLength <= speedCalcCutoff) {
        finalAdjustedSpeed = calculateAdjustedSpeed(finalSegmentLength);  // Adjust speed for the final segment
    }

    E += calculateExtrusionLength(prevX, prevY, polygon[0].x, polygon[0].y);
    gcodeFile << "G1 X" << polygon[0].x << " Y" << polygon[0].y << " E" << E << " F" << finalAdjustedSpeed << "\n"; // Close the loop
    gcodeFile << "G0 F6000 X" << polygon[0].x << " Y " << polygon[0].y << "\n";
}

void GcodeCreator::writeInitializationGCode(std::ofstream& gcodeFile, const std::string& filename) {
    // Write initialization G-code
    gcodeFile << "%Setup: " << filename << "\n";
    gcodeFile << "M140 S" + std::to_string(bedTemp) + "\n"; // Set bed temp
    gcodeFile << "M104 S" + std::to_string(nozzleTemp) + "\n"; // Set nozzle temp
    gcodeFile << "M190 S" + std::to_string(bedTemp) + "\n"; // Wait for bed temp
    gcodeFile << "M109 S" + std::to_string(nozzleTemp) + "\n"; // Wait for nozzle temp
    gcodeFile << "M82\n"; // Absolute extrusion mode
    gcodeFile << "G28\n";       // Home all axes
    gcodeFile << "G92 E0\n";    // Reset extruder

    // Prime the nozzle
    if (prime) {
        gcodeFile << "; Priming" << "\n";
        gcodeFile << "G1 Z2.0 F3000\n";
        gcodeFile << "G1 X0.1 Y20 Z" + std::to_string(layerHeight) + " F5000\n";
        gcodeFile << "G1 X0.1 Y150 Z" + std::to_string(layerHeight) + " F1500 E15\n";
        gcodeFile << "G1 X0.4 Y150 Z" + std::to_string(layerHeight) + " F5000\n";
        gcodeFile << "G1 X0.4 Y20 Z" + std::to_string(layerHeight) + " F1500 E30\n";
        gcodeFile << "G1 Z" + std::to_string(layerHeight) + " F3000\n";
        gcodeFile << "G92 E0\n";
        gcodeFile << "G1 Z2.0 F3000\n";
        gcodeFile << "G1 X5 Y20 Z2 F5000.0\n";
        gcodeFile << "G92 E0\n";
        gcodeFile << "M107\n";  // Turn off fan
    }

    // Adjust initial feed rate
    gcodeFile << "G1 Z" + std::to_string(layerHeight) + " F" << printSpeed << "\n"; // Set feed rate
}



double GcodeCreator::calculateExtrusionLength(double prevX, double prevY, double currentX, double currentY, double factor) {
    double length = sqrt(pow(currentX - prevX, 2) + pow(currentY - prevY, 2));
    double filamentArea = M_PI * pow(filamentDiameter / 2, 2);
    return ((length * layerHeight * nozzleDiameter) / filamentArea);
}

void GcodeCreator::retractionStep(double& E, std::ofstream& gcodeFile, Clipper2Lib::PointD nextPoint) {
    E -= retractionDistance;
    gcodeFile << "G1 E" << E << " F6000\n";

    gcodeFile << "G0 X" << nextPoint.x << " Y" << nextPoint.y << "\n";

    E += retractionDistance;
    gcodeFile << "G1 E" << E << "\n"; // Restore filament dynamically
}


double GcodeCreator::calculateSegmentLength(double x1, double y1, double x2, double y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

double GcodeCreator::calculateAdjustedSpeed(double segmentLength) {
    if (segmentLength > speedCalcCutoff) {
        return printSpeed; // Full speed for long segments
    }
    else if (segmentLength > 0.0) {
		double reductionFactor = minSpeedMultiplier + ((1-minSpeedMultiplier) * (segmentLength / speedCalcCutoff)); // Linear interpolation to be between minSpeedMultiplier and 1.0
        return printSpeed * reductionFactor;
    }
    else {
        return printSpeed * minSpeedMultiplier; // Minimum speed for zero or negative segment length
    }
}