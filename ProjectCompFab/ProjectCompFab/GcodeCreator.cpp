#include "GcodeCreator.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <qmath.h>

GcodeCreator::GcodeCreator() {
	maxXDistance = 0.0;
	maxYDistance = 0.0;
}
void GcodeCreator::generateGCode(const double maxXDist, const double maxYDist, const int sliceAmount, const std::vector<Clipper2Lib::PathsD> erodedSlices,
    const std::vector<std::vector<Clipper2Lib::PathsD>> shells, const std::vector<Clipper2Lib::PathsD> infill,
    const std::string& filename, double layerHeight, double filamentDiameter, double bedTemp, double nozzleTemp, double nozzleDiameter, bool prime) {

    std::ofstream gcodeFile(filename + "\.gcode");

	maxXDistance = maxXDist;
	maxYDistance = maxYDist;

    if (!gcodeFile.is_open()) {
        std::cerr << "Failed to open file for writing.\n";
        return;
    }

    writeInitializationGCode(gcodeFile, bedTemp, nozzleTemp, layerHeight, prime);

    double E = 0.0;
    double retractionDistance = 5.0;
    bool firstPolygon = true;
    bool firstPoint = true;

    for (int slice = 0; slice < sliceAmount; slice++) {
		writeSliceGCode(slice, firstPolygon, firstPoint, E, retractionDistance, gcodeFile, filamentDiameter, layerHeight, nozzleDiameter, erodedSlices, shells, infill);
    }
	E -= retractionDistance;
    // Finish G-code
    gcodeFile << "G1 E" << E << "F2400\n"; // Retract filament
    gcodeFile << "G1 Z10 F3000\n"; // Move nozzle up
    gcodeFile << "G1 X0 Y220.0 Z10 E" << E << "F3000\n";
	gcodeFile << "M106 S0\n";          // Turn off fan
    gcodeFile << "M104 S0\n";      // Turn off hotend
    gcodeFile << "M140 S0\n";      // Turn off bed
    gcodeFile << "M84\n";          // Disable motors
    gcodeFile << "; End of G-code\n";

    gcodeFile.close();
}

void GcodeCreator::writeSliceGCode(int slice, bool& firstPolygon, bool& firstPoint, double& E, double retractionDistance, std::ofstream& gcodeFile, 
    double filamentDiameter, double layerHeight, double nozzleDiameter, const std::vector<Clipper2Lib::PathsD>& erodedSlices,
    const std::vector<std::vector<Clipper2Lib::PathsD>>& shells, const std::vector<Clipper2Lib::PathsD>& infill) {
    gcodeFile << "; Slice " << slice << "\n";
    firstPolygon = true;
    gcodeFile << "G0 Z" << (layerHeight * (slice + 1)) << "\n"; // Move to the current layer

    //ERODED SLICES
    for (const auto& polygon : erodedSlices[slice]) {
        writePolygonGCode(firstPolygon, firstPoint, E, retractionDistance, gcodeFile, polygon, filamentDiameter, layerHeight, nozzleDiameter);
    }

    //SHELLS
    firstPolygon = true;
    for (const auto& setOfShells : shells[slice]) {
        for (const auto& shell : setOfShells) {
            writePolygonGCode(firstPolygon, firstPoint, E, retractionDistance, gcodeFile, shell, filamentDiameter, layerHeight, nozzleDiameter);
        }
    }

    //INFILL
    for (const auto& line : infill[slice]) {
        E -= retractionDistance;
        gcodeFile << "G1 E" << E << " F3000\n";
        gcodeFile << "G0 X" << line[0].x << " Y" << line[0].y << "\n";
        E += retractionDistance;
        gcodeFile << "G1 E" << E << " F800\n"; // Restore filament dynamically
        E += calculateExtrusionLength(line[0].x, line[0].y, line[1].x, line[1].y, filamentDiameter, layerHeight, nozzleDiameter);
        gcodeFile << "G1 X" << line[1].x << " Y" << line[1].y << " E" << E << "\n";
    }
}

void GcodeCreator::writePolygonGCode(bool& firstPolygon, bool& firstPoint, double &E, double retractionDistance, std::ofstream& gcodeFile, 
    const Clipper2Lib::PathD& polygon, double filamentDiameter,double layerHeight, double nozzleDiameter) {
    if (!firstPolygon) {
        E -= retractionDistance;
        gcodeFile << "G1 E" << E << " F3000\n";

        gcodeFile << "G0 X" << polygon[0].x << " Y" << polygon[0].y << "\n";

        E += retractionDistance;
        gcodeFile << "G1 E" << E << " F3000\n"; // Restore filament dynamically
    }
    firstPolygon = false; // After the first polygon
    gcodeFile << "G1 F800\n"; // Set feed rate
    double prevX = polygon[0].x, prevY = polygon[0].y;
    firstPoint = true; // To check if it's the first point in the polygon
    for (const auto& point : polygon) {
        if (!firstPoint) {
            E += calculateExtrusionLength(prevX, prevY, point.x, point.y, filamentDiameter, layerHeight, nozzleDiameter);
        }
        gcodeFile << "G1 X" << point.x << " Y" << point.y << " E" << E << "\n";

        // Update the previous point
        prevX = point.x;
        prevY = point.y;
        firstPoint = false;
    }
    E += calculateExtrusionLength(prevX, prevY, polygon[0].x, polygon[0].y, filamentDiameter, layerHeight, nozzleDiameter);
    gcodeFile << "G1 X" << polygon[0].x << " Y" << polygon[0].y << " E" << E << "\n"; // Close the loop
    gcodeFile << "G0 F6000 X" << polygon[0].x + 0.01 << " Y " << polygon[0].y + 0.01 << "\n";
}

void GcodeCreator::writeInitializationGCode(std::ofstream& gcodeFile, double bedTemp, double nozzleTemp, double layerHeight, bool prime) {
    // Write initialization G-code
    gcodeFile << "%\n";
    gcodeFile << "M140 S" + std::to_string(bedTemp) + "\n"; // Set bed temp
    gcodeFile << "M104 S" + std::to_string(nozzleTemp) + "\n"; // Set nozzle temp
    gcodeFile << "M190 S" + std::to_string(bedTemp) + "\n"; // Wait for bed temp
    gcodeFile << "M109 S" + std::to_string(nozzleTemp) + "\n"; // Wait for nozzle temp
    gcodeFile << "M82\n"; // Absolute extrusion mode
    gcodeFile << "G28\n";       // Home all axes
    gcodeFile << "G92 E0\n";    // Reset extruder

    // Prime the nozzle
    if (prime) {
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
    gcodeFile << "G1 Z" + std::to_string(layerHeight + 0.1) + " F800\n"; // Set feed rate
}



double GcodeCreator::calculateExtrusionLength(double prevX, double prevY, double currentX, double currentY, double filamentDiameter, double layerHeight, double nozzleDiameter, double factor) {
	//maxXDistance and maxYDistance are the maximum distances there can be between the X and Y, these we can use to normalize our distances to print less when    
    // Normalize X and Y distances
    double normalizedAndWeightedX = pow((fabs(currentX - prevX) / maxXDistance),2);
    double normalizedAndWeightedY = pow((fabs(currentY - prevY) / maxYDistance),2);

    // Scale normalized values to range [0.5, 1]
    double scaledNormalizedX = 0.9 + 0.1 * normalizedAndWeightedX;
    double scaledNormalizedY = 0.9 + 0.1 * normalizedAndWeightedY;

    // Combine X and Y scaling factors (use average for simplicity)
    double normalizationFactor = (scaledNormalizedX + scaledNormalizedY) / 2.0;

	factor = normalizationFactor*factor;
    
    
    double length = sqrt(pow(currentX - prevX, 2) + pow(currentY - prevY, 2));
    double filamentArea = M_PI * pow(filamentDiameter / 2, 2);
	return ((length * layerHeight * nozzleDiameter) / filamentArea) * factor;
}