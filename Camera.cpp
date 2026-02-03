#include <GLFW/glfw3.h>
#include "Camera.h"

namespace {
	constexpr float GROUND_HEIGHT = 3.0f;

}

Camera::Camera() :
	position(0.0f, GROUND_HEIGHT, 8.0f), front(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f), yaw(-90.0f),
	pitch(0.0f), speed(20.0f), sensitivity(0.03f), lastX(0.0), lastY(0.0), firstMouse(true),
	mode(CameraMode::GroundLevel), previousMode(CameraMode::GroundLevel), eWasPressed(false),
	spaceWasPressed(false), followYawOffset(0.0f), followPitchOffset(0.0f) {
	trainPoint = { glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), };
	updateVectors();
}

void Camera::update(GLFWwindow* window, float deltaTime) {
	bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	bool ePressed = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

	if (spacePressed && !spaceWasPressed && mode != CameraMode::FreeFly) {
		if (mode == CameraMode::GroundLevel) setMode(CameraMode::FollowTrain);
		else if (mode == CameraMode::FollowTrain) setMode(CameraMode::GroundLevel);
	}

	spaceWasPressed = spacePressed;

	if (ePressed && !eWasPressed) {
		if (mode == CameraMode::FreeFly) {
			setMode(previousMode);
		} else {
			previousMode = mode;
			setMode(CameraMode::FreeFly);
		}
	}
	eWasPressed = ePressed;

	float velocity = speed * deltaTime;

	auto key = [&](int k) { return glfwGetKey(window, k) == GLFW_PRESS; };
	if (mode == CameraMode::GroundLevel) {
		glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
		glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, glm::vec3(0, 1, 0)));

		if (key(GLFW_KEY_W)) position += velocity * flatFront;
		if (key(GLFW_KEY_S)) position -= velocity * flatFront;
		if (key(GLFW_KEY_A)) position -= velocity * flatRight;
		if (key(GLFW_KEY_D)) position += velocity * flatRight;

		position.y = GROUND_HEIGHT;
	} else if (mode == CameraMode::FollowTrain) {
		position = trainPoint.position;

		float pitch = glm::radians(followPitchOffset);
		float yaw = glm::radians(followYawOffset);

		glm::vec3 trainRight = glm::normalize(glm::cross(trainPoint.forward, trainPoint.up));
		glm::vec3 lookDir = trainPoint.forward * cos(yaw) + trainRight * sin(yaw);
		lookDir = glm::normalize(lookDir * cos(pitch) + trainPoint.up * sin(pitch));

		front = lookDir;
		right = glm::normalize(glm::cross(front, trainPoint.up));
		up = glm::normalize(glm::cross(right, front));
	} else if (mode == CameraMode::FreeFly) {
		if (key(GLFW_KEY_W)) position += velocity * front;
		if (key(GLFW_KEY_S)) position -= velocity * front;
		if (key(GLFW_KEY_A)) position -= velocity * right;
		if (key(GLFW_KEY_D)) position += velocity * right;

		position.y = std::max(position.y, 0.15f);
	}
}

void Camera::updateVectors() {
	auto x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	auto y = sin(glm::radians(pitch));
	auto z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(glm::vec3(x, y, z));
	right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::mouseCallback(double x, double y) {
	if (!firstMouse) {
		float dx = float(x - lastX) * sensitivity;
		float dy = float(lastY - y) * sensitivity;

		if (mode == CameraMode::FollowTrain) {
			followYawOffset += dx;
			followPitchOffset += dy;
			followPitchOffset = glm::clamp(followPitchOffset, -45.0f, 45.0f);
		} else {
			yaw += dx;
			pitch += dy;
			pitch = glm::clamp(pitch, -89.0f, 89.0f);
			updateVectors();
		}
	}

	firstMouse = false;
	lastX = x;
	lastY = y;
}

void Camera::setMode(CameraMode newMode) {
	mode = newMode;
	pitch = glm::degrees(asin(front.y));
	yaw = glm::degrees(atan2(front.z, front.x));

	if (mode == CameraMode::GroundLevel) {
		position.y = GROUND_HEIGHT;
	} else if (mode == CameraMode::FollowTrain) {
		followYawOffset = 0.0f;
		followPitchOffset = 0.0f;
	}
}

CameraMode Camera::getMode() const {
	return mode;
}

glm::mat4 Camera::projection(float aspect) const {
	return glm::perspective(glm::radians(60.0f), aspect, 0.1f, 500.0f);
}

glm::mat4 Camera::view() const {
	return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::getPosition() const {
	return position;
}