#version 330 core

in vec3 fragNormal;      // Normal from vertex shader
in vec3 fragPosition;    // Position from vertex shader
out vec4 color;          // Final color output

uniform vec3 lightPos;   // Light position in world space
uniform vec3 viewPos;    // Camera position in world space
uniform vec3 lightColor; // Light color
uniform vec4 cubeColor;  // Base color of the cube (material color)

uniform float shininess; // Shininess factor for specular highlight

void main() {
    vec3 normal = normalize(fragNormal); // Normalize the input normal

    // Ambient lighting
    vec3 ambient = 0.1 * lightColor;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    // Combine lighting effects
    vec3 lighting = ambient + diffuse + specular;
    vec3 finalColor = lighting * vec3(cubeColor);

    color = vec4(finalColor, cubeColor.a);
}
