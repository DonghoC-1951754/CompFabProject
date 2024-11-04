#version 330 core

layout(location = 0) in vec3 position; // Vertex position
layout(location = 1) in vec3 normal;   // Vertex normal

uniform mat4 model;      // Model matrix
uniform mat4 view;       // View matrix
uniform mat4 projection;  // Projection matrix

out vec3 fragNormal;     // Normal to pass to fragment shader
out vec3 fragPosition;   // Position to pass to fragment shader

void main() {
    // Transform the vertex position into clip space
    fragPosition = vec3(model * vec4(position, 1.0));
    fragNormal = normalize(mat3(transpose(inverse(model))) * normal); // Transform normal

    gl_Position = projection * view * vec4(fragPosition, 1.0);
}