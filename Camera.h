#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "InputListener.h"
#include "DataClasses.h"

enum class CameraMode {
	GroundLevel,
	FollowTrain,
	FreeFly
};

class Camera : public MouseListener {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	float sensitivity;
	float speed;
	float yaw;
	float pitch;

	bool firstMouse;
	double lastX;
	double lastY;

	CameraMode mode;
	CameraMode previousMode;
	bool spaceWasPressed;
	bool eWasPressed;

	float followYawOffset;
	float followPitchOffset;

	void updateVectors();

public:
	OrientedPoint trainPoint;

	Camera();

	glm::mat4 view() const;
	glm::mat4 projection(float aspect) const;
	glm::vec3 getPosition() const;

	void mouseCallback(double x, double y) override;
	void update(GLFWwindow* window, float deltaTime);

	void setMode(CameraMode newMode);
	CameraMode getMode() const;
};