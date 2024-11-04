#ifndef OPENGL_PROJECT_TEXTURE_H
#define OPENGL_PROJECT_TEXTURE_H

#include <string>
#include <utility>
#include <assimp/scene.h>

class Texture {
public:
    Texture() {};
    Texture(unsigned int textureID, std::string path, aiTextureType type) : mTextureID(textureID), mPath(std::move(path)), mType(type) {}

    unsigned int getTextureID() const { return mTextureID; };
    aiTextureType getType() const { return mType; };
    std::string getPath() const { return mPath; };

    void setTextureID(unsigned int textureID) { mTextureID = textureID; };
    void setType(aiTextureType type) { mType = type; };
    void setPath(std::string path) { mPath = std::move(path); };

private:
    unsigned int mTextureID;
    aiTextureType mType;
    std::string mPath;
};


#endif //OPENGL_PROJECT_TEXTURE_H
