#include "TrainCar.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <cmath>

namespace {
	constexpr float CAR_LENGTH = 6.0f, CAR_WIDTH = 2.4f, CAR_HEIGHT = 2.8f;

	constexpr float WALL_THICKNESS = 0.08f;
	constexpr float WHEEL_RADIUS = 0.4f, WHEEL_WIDTH = 0.35f;
	constexpr float WHEEL_LENGTH_OFFSET_RATIO = 0.32f, WHEEL_WIDTH_OFFSET_RATIO = 0.85f;
	constexpr int WHEEL_SIDES = 16;

	constexpr float STRIPE_HEIGHT_RATIO = 0.25f;
	constexpr float STRIPE_OFFSET = 0.002f;

	constexpr float SEAT_DEPTH = 1.5f, SEAT_HEIGHT = 1.3f;
	constexpr float SEAT_BACK_HEIGHT = 1.8f, SEAT_BACK_THICKNESS = 0.25f;
	constexpr float FRONT_SEAT_POS_RATIO = 0.3f, BACK_SEAT_POS_RATIO = -0.7f;

	const Color STRIPE_COLOR(1.0f, 0.85f, 0.1f);
	const Color WHEEL_COLOR(0.1f, 0.1f, 0.1f);
	const Color CAR_COLOR(0.85f, 0.1f, 0.1f);
	const Color SEAT_COLOR(0.5f, 0.5f, 0.5f);
}

TrainCar::TrainCar() :
	wallThickness(WALL_THICKNESS), wheelRadius(WHEEL_RADIUS),
	carColor(CAR_COLOR), stripeColor(STRIPE_COLOR),
	wheelColor(WHEEL_COLOR), seatColor(SEAT_COLOR),
	bodyIndicesStart(0), bodyIndicesCount(0),
	stripeIndicesStart(0), stripeIndicesCount(0),
	seatIndicesStart(0), seatIndicesCount(0),
	wheelIndicesStart(0), wheelIndicesCount(0) {
	buildMesh();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

TrainCar::~TrainCar() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void TrainCar::addQuadFace(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 normal) {
	unsigned int base = vertices.size();
	vertices.insert(vertices.end(), { {v0, normal}, {v1, normal}, {v2, normal}, {v3, normal} });
	indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
}

void TrainCar::buildMesh() {
	float bodyHeight = CAR_HEIGHT - wheelRadius;
	float bodyBottom = wheelRadius;
	float halfLen = CAR_LENGTH / 2.0f, halfWidth = CAR_WIDTH / 2.0f;
	float innerHalfLen = halfLen - wallThickness, innerHalfWidth = halfWidth - wallThickness;

	bodyIndicesStart = indices.size();
	buildCarBody(bodyBottom, bodyHeight, halfLen, halfWidth, innerHalfLen, innerHalfWidth);
	bodyIndicesCount = indices.size() - bodyIndicesStart;

	stripeIndicesStart = indices.size();
	buildStripe(bodyBottom, bodyHeight, halfLen, halfWidth, bodyHeight * STRIPE_HEIGHT_RATIO);
	stripeIndicesCount = indices.size() - stripeIndicesStart;

	seatIndicesStart = indices.size();
	buildSeats(bodyBottom, innerHalfLen, innerHalfWidth);
	seatIndicesCount = indices.size() - seatIndicesStart;

	wheelIndicesStart = indices.size();
	buildWheels(bodyBottom);
	wheelIndicesCount = indices.size() - wheelIndicesStart;
}

void TrainCar::buildCarBody(float bodyBottom, float bodyHeight, float halfLen, float halfWidth, float innerHalfLen, float innerHalfWidth) {
	float top = bodyBottom + bodyHeight, innerBottom = bodyBottom + wallThickness;

	glm::vec3 outer[8] = {
		{-halfLen, bodyBottom, -halfWidth}, {halfLen, bodyBottom, -halfWidth},
		{halfLen, bodyBottom, halfWidth}, {-halfLen, bodyBottom, halfWidth},
		{-halfLen, top, -halfWidth}, {halfLen, top, -halfWidth},
		{halfLen, top, halfWidth}, {-halfLen, top, halfWidth}
	};
	glm::vec3 inner[8] = {
		{-innerHalfLen, innerBottom, -innerHalfWidth}, {innerHalfLen, innerBottom, -innerHalfWidth},
		{innerHalfLen, innerBottom, innerHalfWidth}, {-innerHalfLen, innerBottom, innerHalfWidth},
		{-innerHalfLen, top, -innerHalfWidth}, {innerHalfLen, top, -innerHalfWidth},
		{innerHalfLen, top, innerHalfWidth}, {-innerHalfLen, top, innerHalfWidth}
	};

	// Outer walls
	addQuadFace(outer[1], outer[2], outer[6], outer[5], { 1, 0, 0 });
	addQuadFace(outer[3], outer[0], outer[4], outer[7], { -1, 0, 0 });
	addQuadFace(outer[2], outer[3], outer[7], outer[6], { 0, 0, 1 });
	addQuadFace(outer[0], outer[1], outer[5], outer[4], { 0, 0, -1 });

	// Inner walls
	addQuadFace(inner[5], inner[6], inner[2], inner[1], { -1, 0, 0 });
	addQuadFace(inner[7], inner[4], inner[0], inner[3], { 1, 0, 0 });
	addQuadFace(inner[6], inner[7], inner[3], inner[2], { 0, 0, -1 });
	addQuadFace(inner[4], inner[5], inner[1], inner[0], { 0, 0, 1 });

	// Bottom faces
	addQuadFace(outer[0], outer[1], outer[2], outer[3], { 0, -1, 0 });
	addQuadFace(inner[2], inner[1], inner[0], inner[3], { 0, 1, 0 });

	// Top rim
	addQuadFace(outer[4], outer[5], inner[5], inner[4], { 0, 1, 0 });
	addQuadFace(outer[6], outer[7], inner[7], inner[6], { 0, 1, 0 });
	addQuadFace(outer[7], outer[4], inner[4], inner[7], { 0, 1, 0 });
	addQuadFace(outer[5], outer[6], inner[6], inner[5], { 0, 1, 0 });
}

void TrainCar::buildStripe(float bodyBottom, float bodyHeight, float halfLen, float halfWidth, float stripeHeight) {
	float mid = bodyBottom + bodyHeight * 0.5f;
	float bot = mid - stripeHeight / 2.0f, top = mid + stripeHeight / 2.0f;
	float len = halfLen + STRIPE_OFFSET, wid = halfWidth + STRIPE_OFFSET;

	addQuadFace({ len, bot, -wid }, { len, bot, wid }, { len, top, wid }, { len, top, -wid }, { 1, 0, 0 });
	addQuadFace({ -len, bot, wid }, { -len, bot, -wid }, { -len, top, -wid }, { -len, top, wid }, { -1, 0, 0 });
	addQuadFace({ len, bot, wid }, { -len, bot, wid }, { -len, top, wid }, { len, top, wid }, { 0, 0, 1 });
	addQuadFace({ -len, bot, -wid }, { len, bot, -wid }, { len, top, -wid }, { -len, top, -wid }, { 0, 0, -1 });
}

void TrainCar::buildSeats(float bodyBottom, float innerHalfLen, float innerHalfWidth) {
	float seatY = bodyBottom + wallThickness;
	float seatPositions[] = { innerHalfLen * FRONT_SEAT_POS_RATIO, innerHalfLen * BACK_SEAT_POS_RATIO };

	for (float xPos : seatPositions) {
		addBox({ xPos - SEAT_DEPTH / 2, seatY, -innerHalfWidth }, { xPos + SEAT_DEPTH / 2, seatY + SEAT_HEIGHT, innerHalfWidth });
		addBox({ xPos - SEAT_DEPTH / 2, seatY + SEAT_HEIGHT, -innerHalfWidth }, { xPos - SEAT_DEPTH / 2 + SEAT_BACK_THICKNESS, seatY + SEAT_HEIGHT + SEAT_BACK_HEIGHT, innerHalfWidth });
	}
}

void TrainCar::buildWheels(float bodyBottom) {
	float lengthOffset = CAR_LENGTH * WHEEL_LENGTH_OFFSET_RATIO;
	float widthOffset = CAR_WIDTH * WHEEL_WIDTH_OFFSET_RATIO;
	float zVariants[] = { -widthOffset / 2.0f, widthOffset / 2.0f };
	float xVariants[] = { lengthOffset, -lengthOffset };

	for (float xPos : xVariants)
		for (float zPos : zVariants)
			addHalfCylinder({ xPos, bodyBottom, zPos }, { 0, 0, 1 }, wheelRadius, WHEEL_WIDTH, WHEEL_SIDES);
}

void TrainCar::addBox(glm::vec3 min, glm::vec3 max) {
	glm::vec3 verts[8] = {
		{min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z},
		{min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}
	};
	addQuadFace(verts[1], verts[2], verts[6], verts[5], { 1, 0, 0 });
	addQuadFace(verts[3], verts[0], verts[4], verts[7], { -1, 0, 0 });
	addQuadFace(verts[2], verts[3], verts[7], verts[6], { 0, 0, 1 });
	addQuadFace(verts[0], verts[1], verts[5], verts[4], { 0, 0, -1 });
	addQuadFace(verts[4], verts[5], verts[6], verts[7], { 0, 1, 0 });
	addQuadFace(verts[0], verts[3], verts[2], verts[1], { 0, -1, 0 });
}

void TrainCar::addHalfCylinder(const glm::vec3& center, const glm::vec3& axis, float radius, float len, int sides) {
	glm::vec3 axisNorm = glm::normalize(axis);
	glm::vec3 start = center - axisNorm * (len / 2.0f), end = center + axisNorm * (len / 2.0f);
	glm::vec3 perp1(1, 0, 0), perp2(0, -1, 0);

	unsigned baseIdx = vertices.size();
	for (int i = 0; i <= sides; ++i) {
		float angle = i * 3.14159f / sides;
		glm::vec3 offset = perp1 * cos(angle) * radius + perp2 * sin(angle) * radius;
		glm::vec3 normal = glm::normalize(offset);
		vertices.push_back({ start + offset, normal });
		vertices.push_back({ end + offset, normal });
	}

	for (int i = 0; i < sides; ++i) {
		unsigned b0 = baseIdx + i * 2, b1 = b0 + 1, b2 = baseIdx + (i + 1) * 2, b3 = b2 + 1;
		indices.insert(indices.end(), { b0, b2, b1, b1, b2, b3 });
	}

	addQuadFace(start + perp1 * radius, start - perp1 * radius, end - perp1 * radius, end + perp1 * radius, { 0, 1, 0 });

	for (int cap = 0; cap < 2; ++cap) {
		glm::vec3 capCenter = cap ? end : start;
		glm::vec3 capNormal = axisNorm * (cap ? 1.0f : -1.0f);
		unsigned capBaseIdx = vertices.size();
		vertices.push_back({ capCenter, capNormal });
		for (int i = 0; i <= sides; ++i) {
			float angle = i * 3.14159f / sides;
			vertices.push_back({ capCenter + perp1 * cos(angle) * radius + perp2 * sin(angle) * radius, capNormal });
		}
		for (int i = 0; i < sides; ++i)
			indices.insert(indices.end(), cap ? std::initializer_list<unsigned>{capBaseIdx, capBaseIdx + i + 2, capBaseIdx + i + 1}
		: std::initializer_list<unsigned>{ capBaseIdx, capBaseIdx + i + 1, capBaseIdx + i + 2 });
	}
}

void TrainCar::draw(const Shader& shader, const glm::vec3& position, const glm::vec3& perp, float pitch) const {
	glm::vec3 right = glm::normalize(perp);
	glm::vec3 forward = -glm::normalize(glm::vec3(-right.z * cos(-pitch), sin(-pitch), right.x * cos(-pitch)));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	glm::mat4 model(1.0f);
	model[0] = glm::vec4(forward, 0.0f);
	model[1] = glm::vec4(up, 0.0f);
	model[2] = glm::vec4(right, 0.0f);
	model[3] = glm::vec4(position, 1.0f);

	shader.setMat4("model", glm::value_ptr(model));
	shader.setBool("useTexture", false);
	glBindVertexArray(VAO);

	struct { const Color& color; unsigned start, count; } parts[] = {
		{carColor, bodyIndicesStart, bodyIndicesCount},
		{stripeColor, stripeIndicesStart, stripeIndicesCount},
		{seatColor, seatIndicesStart, seatIndicesCount},
		{wheelColor, wheelIndicesStart, wheelIndicesCount}
	};

	for (const auto& part : parts) {
		shader.setVec3("baseColor", part.color.red, part.color.green, part.color.blue);
		glDrawElements(GL_TRIANGLES, part.count, GL_UNSIGNED_INT, (void*)(part.start * sizeof(unsigned int)));
	}

	glBindVertexArray(0);
}