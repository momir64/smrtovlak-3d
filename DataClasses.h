#pragma once
#include <glm/glm.hpp>

class Color {
public:
	float red;
	float green;
	float blue;

	Color(float red = 0, float green = 0, float blue = 0);
	Color(int red, int green, int blue);
};

class Bounds {
public:
	float x;
	float y;
	float width;
	float height;
	float angle;
	bool flip;

	Bounds(float x, float y, float width, float height = 0, float angle = 0, bool flip = false);
};

struct TrackPoint {
	float distance = 0.0f;
	float pitch = 0.0f;
	glm::vec3 center;
	glm::vec3 perp;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct OrientedPoint {
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
};