#pragma once
#include <glm/glm.hpp>

struct Bounds {
	float x;
	float y;
	float width;
	float height = 0;
	float angle = 0;
	bool flip = false;
};

struct TrackPoint {
	float distance = 0.0f;
	float pitch = 0.0f;
	glm::vec3 center;
	glm::vec3 perp;
};

struct OrientedPoint {
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};