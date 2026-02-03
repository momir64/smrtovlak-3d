#pragma once
#include "WindowManager.h"
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Ground.h"
#include "Tracks.h"
#include "Train.h"
#include "Text.h"

class Smrtovlak : public ResizeListener {
    WindowManager window;
    Camera camera;
    Shader shader;
    Ground ground;
    Tracks tracks;
    Train train;
    Text text;

    bool numberKeysWasPressed[8] = { false };

public:
    Smrtovlak();

    int run();
    void draw();
    void resizeCallback(GLFWwindow& window) override;
};