#version 330 core

in vec3 fragNormal;       // Normal from vertex shader
out vec4 color;           // Final color output

uniform vec4 cubeColor;   // Uniform color variable

void main() {
    // Simple shading: just use the uniform color
    color = cubeColor; // Set the output color to the uniform color
}