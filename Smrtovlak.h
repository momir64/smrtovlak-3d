#pragma once
#include "WindowManager.h"
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Ground.h"
#include "Tracks.h"
#include "TrainCar.h"
#include "Train.h"

class Smrtovlak : public ResizeListener {
    WindowManager window;
    Camera camera;
    Shader shader;
    Ground ground;
    Tracks tracks;
    Train train;

    glm::vec3 lightColor;
    glm::vec3 lightPos;

public:
    Smrtovlak();

    int run();
    void draw();
    void resizeCallback(GLFWwindow& window) override;
};
