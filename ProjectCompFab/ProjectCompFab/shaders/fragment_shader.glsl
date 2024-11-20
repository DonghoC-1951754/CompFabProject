#version 330 core

out vec4 FragColor;

in vec3 fragPos;  // World-space position of the fragment
in vec3 normal;   // Normal vector at the fragment

uniform vec3 lightPos;      // Light position in world space
uniform vec3 lightColor;    // Light color
uniform vec3 objectColor;   // Object color

void main()
{
    // Normalize the normal and light direction
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);  // Direction from fragment to light

    // Diffuse lighting (Lambertian reflection)
    float diff = max(dot(norm, lightDir), 0.0);  // Lambert's cosine law
    vec3 diffuse = diff * lightColor * objectColor;

    // Final color
    FragColor = vec4(diffuse, 1.0);
}
