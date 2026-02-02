#pragma once
#include "DataClasses.h"
#include <glm/glm.hpp>
#include "Shader.h"
#include <vector>

class TrainCar {
	unsigned int VAO = 0, VBO = 0, EBO = 0;
	std::vector<unsigned int> indices;
	std::vector<Vertex> vertices;

	// Car dimensions
	float wallThickness;
	float wheelRadius;
	Color carColor;
	Color stripeColor;
	Color wheelColor;
	Color seatColor;

	// Index ranges for different colored parts
	unsigned int bodyIndicesStart;
	unsigned int bodyIndicesCount;
	unsigned int stripeIndicesStart;
	unsigned int stripeIndicesCount;
	unsigned int seatIndicesStart;
	unsigned int seatIndicesCount;
	unsigned int wheelIndicesStart;
	unsigned int wheelIndicesCount;

	void buildMesh();
	void buildCarBody(float bodyBottom, float bodyHeight, float halfLen, float halfWidth, float innerHalfLen, float innerHalfWidth);
	void buildStripe(float bodyBottom, float bodyHeight, float halfLen, float halfWidth, float stripeHeight);
	void buildSeats(float bodyBottom, float innerHalfLen, float innerHalfWidth);
	void buildWheels(float bodyBottom);

	void addQuadFace(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n);
	void addHalfCylinder(const glm::vec3& center, const glm::vec3& axis, float radius, float length, int sides = 16);
	void addBox(glm::vec3 minCorner, glm::vec3 maxCorner);

public:
	TrainCar();
	~TrainCar();

	void draw(const Shader& shader, const glm::vec3& position, const glm::vec3& perp, float pitch) const;
};