#include "Smrtovlak.h"
#include <glm/glm.hpp>
#include <thread>
#include <chrono>

Smrtovlak::Smrtovlak()
	: window(1280, 800, 800, 600, "Smrtovlak 3D", "assets/icons/icon.png", false),
	shader("shaders/3d.vert", "shaders/3d.frag"),
	ground("assets/textures/grass.jpg"),
	tracks("smrtovlak.track"),
	car(),
	lightColor(1.0f, 0.95f, 0.6f),
	lightPos(30.0f, 50.0f, 5.0f) {

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

	window.setResizeListener(this);
	window.addMouseListener(&camera);

	glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

static int i = -50;

void Smrtovlak::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int height = window.getHeight();
	float aspect = (height == 0) ? 1.0f : (float)window.getWidth() / height;
	glm::vec3 viewPos = camera.getPosition();

	shader.use();
	shader.setMat4("view", &camera.view()[0][0]);
	shader.setMat4("projection", &camera.projection(aspect)[0][0]);

	shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);
	shader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
	shader.setVec3("viewPos", viewPos.x, viewPos.y, viewPos.z);

	ground.draw(shader);
	tracks.draw(shader);

	for (int j = 0; j < 4; j++) {
		int x = (tracks.points.size() + i - 400 * j) % tracks.points.size();
		car.draw(shader, tracks.points[x].center, tracks.points[x].perp, tracks.points[x].pitch);
	}

	i = (i + 20) % tracks.points.size();

	window.swapBuffers();
	glfwPollEvents();
}

int Smrtovlak::run() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

	const double targetFrame = 1.0 / 75.0; // 75 FPS
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(startTime - lastTime).count();
		lastTime = startTime;

		camera.update(window.getWindow(), deltaTime);
		draw();

		auto endTime = std::chrono::high_resolution_clock::now();
		double elapsed = std::chrono::duration<double>(endTime - startTime).count();
		double remaining = targetFrame - elapsed;
		if (remaining > 0)
			std::this_thread::sleep_for(std::chrono::duration<double>(remaining));
	}

	return 0;
}

void Smrtovlak::resizeCallback(GLFWwindow&) {
	draw();
}
