#include "WindowManager.h"
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

WindowManager::WindowManager(int width, int height, int minWidth, int minHeight, const std::string& title, const std::string& iconPath, bool fullscreen) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int monitorHeight = mode->height;
	int monitorWidth = mode->width;

	this->fullscreen = fullscreen;
	yPos = (monitorHeight - height) / 2;
	xPos = (monitorWidth - width) / 2;
	lastHeight = height;
	lastWidth = width;

	if (fullscreen) {
		window = glfwCreateWindow(monitorWidth, monitorHeight, title.c_str(), monitor, NULL);
		this->height = monitorHeight;
		this->width = monitorWidth;
	} else {
		window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		glfwSetWindowPos(window, xPos, yPos);
		this->height = height;
		this->width = width;
	}

	if (!window) {
		std::cerr << "Error: unable to create the window..." << std::endl;
		exit(-1);
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewInit()) << std::endl;
		exit(-1);
	}

	glfwSetWindowSizeLimits(window, minWidth, minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

	GLFWimage icon{};
	icon.pixels = stbi_load(iconPath.c_str(), &icon.width, &icon.height, 0, 4);
	if (icon.pixels)
		glfwSetWindowIcon(window, 1, &icon);
	stbi_image_free(icon.pixels);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, keyboardEventHandler);
	glfwSetCursorPosCallback(window, mouseEventHandler);
	glfwSetFramebufferSizeCallback(window, resizeEventHandler);

	addKeyboardListener(this);
}

WindowManager::~WindowManager() {
	glfwTerminate();
}

GLFWmonitor* WindowManager::getMonitor() {
	int monitorCount, bestArea = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
	glfwGetWindowPos(window, &xPos, &yPos);

	for (int i = 0; i < monitorCount; i++) {
		int monX, monY, monW, monH;
		glfwGetMonitorWorkarea(monitors[i], &monX, &monY, &monW, &monH);

		int overlapW = std::max(0, std::min(xPos + width, monX + monW) - std::max(xPos, monX));
		int overlapH = std::max(0, std::min(yPos + height, monY + monH) - std::max(yPos, monY));
		int overlapArea = overlapW * overlapH;

		if (overlapArea > bestArea)
			bestArea = overlapArea, bestMonitor = monitors[i];
	}

	return bestMonitor;
}

void WindowManager::setFullscreen(bool fullscreen) {
	this->fullscreen = fullscreen;

	if (fullscreen) {
		glfwGetWindowSize(window, &lastWidth, &lastHeight);
		GLFWmonitor* monitor = getMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	} else {
		glfwSetWindowMonitor(window, NULL, xPos, yPos, lastWidth, lastHeight, 0);
	}
}

WindowManager& WindowManager::getWindowManager(GLFWwindow* window) {
	auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
	if (!self) {
		std::cerr << "Error: WindowManager instance not found for the given GLFWwindow!" << std::endl;
		std::exit(-1);
	}
	return *self;
}

void WindowManager::keyboardEventHandler(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto& wm = getWindowManager(window);
	for (auto& listener : wm.keyboardListeners)
		listener->keyboardCallback(*window, key, scancode, action, mods);
}

void WindowManager::mouseEventHandler(GLFWwindow* window, double x, double y) {
	auto& wm = getWindowManager(window);
	for (auto* listener : wm.mouseListeners)
		listener->mouseCallback(x, y);
}

void WindowManager::resizeEventHandler(GLFWwindow* window, int newWidth, int newHeight) {
	auto& wm = getWindowManager(window);
	wm.width = newWidth;
	wm.height = newHeight;
	glViewport(0, 0, newWidth, newHeight);
	if (wm.resizeListener)
		wm.resizeListener->resizeCallback(*window);
}

bool WindowManager::shouldClose() { return glfwWindowShouldClose(window); }
void WindowManager::swapBuffers() { glfwSwapBuffers(window); }
int WindowManager::getHeight() const { return height; }
int WindowManager::getWidth() const { return width; }

void WindowManager::addKeyboardListener(KeyboardListener* listener) {
	keyboardListeners.push_back(listener);
}

void WindowManager::setResizeListener(ResizeListener* listener) {
	resizeListener = listener;
}

void WindowManager::addMouseListener(MouseListener* listener) {
	mouseListeners.push_back(listener);
}

void WindowManager::keyboardCallback(GLFWwindow& window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		setFullscreen(!fullscreen);
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		exit(0);
}
