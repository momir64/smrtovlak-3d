#pragma once
#include <GLFW/glfw3.h>

class Button;

class MouseListener {
public:
	virtual void mouseCallback(double x, double y) = 0;
};

class KeyboardListener {
public:
	virtual void keyboardCallback(GLFWwindow& window, int key, int scancode, int action, int mods) = 0;
};

class ResizeListener {
public:
	virtual void resizeCallback(GLFWwindow& window) = 0;
};