#include "Smrtovlak.h"
#include <thread>
#include <chrono>

namespace {
	constexpr float LIGHT_X = 30.0f, LIGHT_Y = 50.0f, LIGHT_Z = 5.0f;
	constexpr glm::vec3 SKY_COLOR = glm::vec3(95, 188, 235) / 255.0f;
	constexpr glm::vec3	LIGHT_COLOR(1.0f, 0.95f, 0.6f);
}

Smrtovlak::Smrtovlak()
	: window(1280, 800, 800, 600, "Smrtovlak 3D", "assets/icons/icon.png", true),
	text(window, L"Momir Stanišić SV39/2022", Bounds(46, 68, 18)),
	shader("shaders/3d.vert", "shaders/3d.frag"),
	ground("assets/textures/grass.jpg"),
	tracks("smrtovlak.track"),
	train(tracks) {

	glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);

	window.setResizeListener(this);
	window.addKeyboardListener(this);
	window.addMouseListener(&camera);

	glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Smrtovlak::keyboardCallback(GLFWwindow& win, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	CameraMode cameraMode = camera.getMode();
	TrainMode trainMode = train.getMode();

	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
		if (trainMode == TrainMode::WAITING) {
			train.buckleUp(key - GLFW_KEY_1);
		} else if (trainMode == TrainMode::RUNNING) {
			train.makeSick(key - GLFW_KEY_1);
			if (key - GLFW_KEY_1 == 0)
				greenTintEnabled = true;
		}
	} else if (key == GLFW_KEY_SPACE && trainMode == TrainMode::WAITING) {
		if (cameraMode == CameraMode::GroundLevel)
			camera.setMode(CameraMode::FollowTrain);
		train.addCharacter();
	} else if (key == GLFW_KEY_E) {
		if (cameraMode != CameraMode::FreeFly)
			camera.setMode(CameraMode::FreeFly);
		else if (train.getCharactersCount() > 0)
			camera.setMode(CameraMode::FollowTrain);
		else
			camera.setMode(CameraMode::GroundLevel);
	} else if (key == GLFW_KEY_ENTER && trainMode == TrainMode::WAITING) {
		train.start();
	}
}

void Smrtovlak::draw() {
	bool cameraInTrain = camera.getMode() == CameraMode::FollowTrain;

	if (greenTintEnabled && cameraInTrain)
		glClearColor(SKY_COLOR.r * 0.6f, SKY_COLOR.g * 1.1f, SKY_COLOR.b * 0.5f, 1.0f);
	else
		glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int height = window.getHeight();
	float aspect = (height == 0) ? 1.0f : (float)window.getWidth() / height;
	glm::vec3 viewPos = camera.getPosition();

	shader.use();
	shader.setMat4("view", &camera.view()[0][0]);
	shader.setMat4("projection", &camera.projection(aspect)[0][0]);

	shader.setVec3("lightColor", LIGHT_COLOR.r, LIGHT_COLOR.g, LIGHT_COLOR.b);
	shader.setVec3("lightPos", LIGHT_X, LIGHT_Y, LIGHT_Z);
	shader.setVec3("viewPos", viewPos.x, viewPos.y, viewPos.z);

	shader.setBool("screenGreenTint", greenTintEnabled && cameraInTrain);
	shader.setVec2("resolution", (float)window.getWidth(), (float)window.getHeight());

	ground.draw(shader);
	tracks.draw(shader);
	train.draw(shader, cameraInTrain);

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

	glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);

	const double targetFrame = 1.0 / 75.0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(startTime - lastTime).count();
		lastTime = startTime;

		train.update(deltaTime);

		if (train.getMode() == TrainMode::FINISHED) {
			train.setMode(TrainMode::WAITING);
			greenTintEnabled = false;
			camera.reset();
		}

		camera.trainPoint = train.getCameraTransform();
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