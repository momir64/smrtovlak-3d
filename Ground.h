#pragma once
#include "Shader.h"
#include <string>

class Ground {
    unsigned int VAO, VBO, EBO;
    unsigned int texture;

    void initMesh();
    void loadTexture(const std::string& path);

public:
    Ground(const std::string& texturePath);
    void draw(const Shader& shader) const;
};
