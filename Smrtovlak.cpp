#include "Smrtovlak.h"
#include <thread>
#include <chrono>

namespace {
	const Color SKY_COLOR(95, 188, 235), LIGHT_COLOR(1.0f, 0.95f, 0.6f);
	constexpr float LIGHT_X = 30.0f, LIGHT_Y = 50.0f, LIGHT_Z = 5.0f;
}

Smrtovlak::Smrtovlak()
	: window(1280, 800, 800, 600, "Smrtovlak 3D", "assets/icons/icon.png", false),
	text(window, L"Momir Stanišić SV39/2022", Bounds(46, 68, 18)),
	shader("shaders/3d.vert", "shaders/3d.frag"),
	ground("assets/textures/grass.jpg"),
	tracks("smrtovlak.track"),
	train(tracks) {

	glEnable(GL_DEPTH_TEST);
	glClearColor(SKY_COLOR.red, SKY_COLOR.green, SKY_COLOR.blue, 1.0f);

	window.setResizeListener(this);
	window.addMouseListener(&camera);

	glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Smrtovlak::draw() {
	if (greenTintEnabled) glClearColor(SKY_COLOR.red * 0.6f, SKY_COLOR.green * 1.1f, SKY_COLOR.blue * 0.5f, 1.0f);
	else glClearColor(SKY_COLOR.red, SKY_COLOR.green, SKY_COLOR.blue, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int height = window.getHeight();
	float aspect = (height == 0) ? 1.0f : (float)window.getWidth() / height;
	glm::vec3 viewPos = camera.getPosition();

	shader.use();
	shader.setMat4("view", &camera.view()[0][0]);
	shader.setMat4("projection", &camera.projection(aspect)[0][0]);

	shader.setVec3("lightColor", LIGHT_COLOR.red, LIGHT_COLOR.green, LIGHT_COLOR.blue);
	shader.setVec3("lightPos", LIGHT_X, LIGHT_Y, LIGHT_Z);
	shader.setVec3("viewPos", viewPos.x, viewPos.y, viewPos.z);

	shader.setBool("screenGreenTint", greenTintEnabled);
	shader.setVec2("resolution", (float)window.getWidth(), (float)window.getHeight());

	ground.draw(shader);
	tracks.draw(shader);
	train.draw(shader);

	text.draw();

	window.swapBuffers();
	glfwPollEvents();
}

int Smrtovlak::run() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glClearColor(SKY_COLOR.red, SKY_COLOR.green, SKY_COLOR.blue, 1.0f);

	const double targetFrame = 1.0 / 75.0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(startTime - lastTime).count();
		lastTime = startTime;

		for (int i = 0; i < 8; ++i) {
			bool keyPressed = glfwGetKey(window.getWindow(), GLFW_KEY_1 + i) == GLFW_PRESS;
			if (keyPressed && !numberKeysWasPressed[i]) {
				train.toggleBelt(i + 1);
			}
			numberKeysWasPressed[i] = keyPressed;
		}

		bool gPressed = glfwGetKey(window.getWindow(), GLFW_KEY_G) == GLFW_PRESS;
		if (gPressed && !gWasPressed) {
			greenTintEnabled = !greenTintEnabled;
		}
		gWasPressed = gPressed;

		train.update(deltaTime);
		camera.trainPoint = train.getFrontCarTransform();
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