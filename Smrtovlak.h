#pragma once
#include "WindowManager.h"
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Ground.h"
#include "Tracks.h"
#include "TrainCar.h"

class Smrtovlak : public ResizeListener {
    WindowManager window;
    Camera camera;
    Shader shader;
    Ground ground;
    Tracks tracks;
    TrainCar car;

    glm::vec3 lightColor;
    glm::vec3 lightPos;

public:
    Smrtovlak();

    int run();
    void draw();
    void resizeCallback(GLFWwindow& window) override;
};
