#version 330 core

in vec2 normalizedCoords;  // Normalized coordinates (0-1 range)
out vec4 FragColor;

uniform float plateWidth;   // Plate width in cm
uniform float plateDepth;   // Plate depth in cm

void main()
{
    float lineThickness = 0.05;  // Gridline thickness in cm

    // Convert normalized coordinates to world space (in cm)
    vec2 worldCoordsCM = normalizedCoords * vec2(plateWidth, plateDepth);

    // Scale the coordinates by 1/10 so the grid is spaced every 1 cm (instead of 1 mm)
    vec2 scaledCoords = worldCoordsCM / 10.0;

    // Calculate the gridlines using modulo for 1 cm intervals
    float distToXGrid = mod(scaledCoords.x, 1.0);  // Gridline spacing is 1.0 (1 cm)
    float distToYGrid = mod(scaledCoords.y, 1.0);  // Gridline spacing is 1.0 (1 cm)

    // If the current fragment is within the line thickness, paint it as a gridline
    if (distToXGrid < lineThickness || distToYGrid < lineThickness) {
        FragColor = vec4(0.2, 0.2, 0.2, 0.8);  // Black gridline
    } else {
        FragColor = vec4(0.8, 0.8, 0.8, 1.0);  // Light gray background
    }
}
