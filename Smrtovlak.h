#pragma once
#include "WindowManager.h"
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Ground.h"
#include "Tracks.h"
#include "Train.h"
#include "Text.h"

class Smrtovlak : public ResizeListener, public KeyboardListener {
    WindowManager window;
    Camera camera;
    Shader shader;
    Ground ground;
    Tracks tracks;
    Train train;
    Text text;

    bool greenTintEnabled = false;

public:
    Smrtovlak();

    int run();
    void draw();
    void resizeCallback(GLFWwindow& window) override;
    void keyboardCallback(GLFWwindow& window, int key, int scancode, int action, int mods) override;
};