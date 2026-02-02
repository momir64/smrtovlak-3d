#include <GLFW/glfw3.h>
#include "Camera.h"

Camera::Camera() :
	position(0.0f, 3.0f, 8.0f),
	front(0.0f, 0.0f, -1.0f),
	up(0.0f, 1.0f, 0.0f),
	yaw(-90.0f),
	pitch(0.0f),
	speed(20.0f),
	sensitivity(0.03f),
	lastX(0.0),
	lastY(0.0),
	firstMouse(true) {
	updateVectors();
}

void Camera::update(GLFWwindow* window, float deltaTime) {
	float velocity = speed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += velocity * front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= velocity * front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position -= velocity * right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += velocity * right;
	position.y = std::max(position.y, 0.15f);
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
		yaw += float(x - lastX) * sensitivity;
		pitch += float(lastY - y) * sensitivity;
		pitch = glm::clamp(pitch, -89.0f, 89.0f);
		updateVectors();
	}

	firstMouse = false;
	lastX = x;
	lastY = y;
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
