#version 330 core

layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal

out vec3 fragPos;  // Position of the fragment (for light calculations)
out vec3 normal;   // Normal vector for diffuse lighting

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));  // World-space position of the vertex
    normal = mat3(transpose(inverse(model))) * aNormal;  // Correct normal transformation

    gl_Position = projection * view * vec4(fragPos, 1.0);  // Transform vertex position into clip space
}
