#include "GcodeCreator.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <qmath.h>

GcodeCreator::GcodeCreator() {
}
void GcodeCreator::generateGCode(const int sliceAmount, const std::vector<Clipper2Lib::PathsD> erodedSlices, const std::vector<Clipper2Lib::PathsD> shells, const std::vector<Clipper2Lib::PathsD> infill,
    const std::string& filename, double layerHeight, double filamentDiameter, double bedTemp, double nozzleTemp, double nozzleDiameter, bool prime) {
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
		gcodeFile << "G92 E0\n";
		gcodeFile << " G1 Z2.0 F3000\n";
		gcodeFile << "G1 X5 Y20 Z2 F5000.0\n";
        gcodeFile << "G92 E0\n";
        gcodeFile << "G92 E0\n";
        gcodeFile << "M107\n";
    }
    

    // Actual printing
    gcodeFile << "G1 Z" + std::to_string(layerHeight+0.1) + " F1200\n"; // Set feed rate
    // OUR Y BECOMES THE Z IN THE G-CODE, MEANING WE USE Y FOR HEIGHT WHILE THE G-CODE USES Z. AND VICE VERSA

    /**
    * TODO: Start with omtrek for print test
    * TODO: Start with omtrek for print test
    * TODO: Start with omtrek for print test
    * TODO: Start with omtrek for print test
    * TODO: Start with omtrek for print test
    */ 
    double E = 0.0;
	double currentLayer = 0;
    
    for (const auto& vertex : polygon) {
        const glm::vec3 startEdge = vertex[0];
        const glm::vec3 endEdge = vertex[1];

        double distance = glm::distance(startEdge, endEdge);
        //extrustionLength = (Layer height x Nozzle diameter x L)/Filament area 

        double extrusionLength = (distance * layerHeight * nozzleDiameter) / M_PI * pow(nozzleDiameter / 2, 2);

        E += distance * 0.55; // Adjust extrusion multiplier as needed

        gcodeFile << std::fixed << std::setprecision(4);
        gcodeFile << "G1 X" << endEdge.x << " Y" << endEdge.z << " E" << E << "\n";
    }

    for (int i = 0; i < sliceAmount; i++) {
        // Write the G-code for the current slice
        gcodeFile << "; Slice " << i << "\n";
        for (const auto& polygon : erodedSlices[i]) {
            gcodeFile << "G1 Z" << (layerHeight * i)+0.1 << "\n"; // Move to the current layer
            double prevX = 0.0, prevY = 0.0;
            bool firstPoint = true; // To check if it's the first point in the polygon
            for (const auto& point : polygon) {
				if (firstPoint) {
                    E += calculateExtrusionLength(prevX, prevY, point.x, point.y, filamentDiameter, layerHeight, nozzleDiameter);
				}
                gcodeFile << "G1 X" << point.x << " Y" << point.y << " E" << E << "\n";

                // Update the previous point
                prevX = point.x;
                prevY = point.y;
                firstPoint = false;
            }
			E += calculateExtrusionLength(prevX, prevY, polygon[0].x, polygon[0].y, filamentDiameter, layerHeight, nozzleDiameter);
			gcodeFile << "G1 X" << polygon[0].x << " Y" << polygon[0].y << "\n"; // Close the loop
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

double GcodeCreator::calculateExtrusionLength(double prevX, double prevY, double currentX, double currentY, double filamentDiameter, double layerHeight, double nozzleDiameter) {
    double length = sqrt(pow(currentX - prevX, 2) + pow(currentY - prevY, 2));
    double filamentArea = M_PI * pow(filamentDiameter / 2, 2);
	return (length * layerHeight * nozzleDiameter) / filamentArea;
}