#ifndef OPENGL_PROJECT_VERTEX_H
#define OPENGL_PROJECT_VERTEX_H

#include <vector>
#include <glm/glm.hpp>

class Vertex {
public:
    Vertex(glm::vec3 position) : Position(position){}
    Vertex() = default;

    glm::vec3 getPosition() const { return Position; }
    glm::vec3 getNormal() const { return Normal; }
    glm::vec2 getTexCoords() const { return TextureCoords; }

    void setPosition(glm::vec3 position) { Position = position; }
    void setNormal(glm::vec3 normal) { Normal = normal; }
    void setTextureCoords(glm::vec2 texCoords) { TextureCoords = texCoords; }

    static size_t offsetPosition() { return offsetof(Vertex, Position); }
    static size_t offsetNormal() { return offsetof(Vertex, Normal); }
    static size_t offsetTextureCoords() { return offsetof(Vertex, TextureCoords); }
    glm::vec3 Normal;
    glm::vec2 TextureCoords;
private:
    glm::vec3 Position;


};


#endif //OPENGL_PROJECT_VERTEX_H
