#version 330 core

layout(location = 0) in vec3 aPos;  // Local position
out vec2 normalizedCoords;          // Normalized (0-1) coordinates of the plane

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float plateWidth;           // Dynamic width (cm)
uniform float plateDepth;           // Dynamic depth (cm)

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Map to normalized coordinates (0 to 1 range)
    normalizedCoords = vec2(aPos.x / plateWidth, aPos.z / plateDepth);
}
