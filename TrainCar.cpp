#include "TrainCar.h"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <numbers>
#include <cmath>

namespace {
	constexpr float CAR_LENGTH = 6.0f, CAR_WIDTH = 2.4f, CAR_HEIGHT = 2.8f;

	constexpr float WHEEL_LENGTH_OFFSET_RATIO = 0.32f, WHEEL_WIDTH_OFFSET_RATIO = 0.85f;
	constexpr float WHEEL_RADIUS = 0.4f, WHEEL_WIDTH = 0.35f;
	constexpr float WALL_THICKNESS = 0.08f;
	constexpr int WHEEL_SIDES = 16;

	constexpr float STRIPE_HEIGHT_RATIO = 0.25f, STRIPE_OFFSET = 0.01f;

	constexpr float FRONT_SEAT_POS_RATIO = 0.3f, BACK_SEAT_POS_RATIO = -0.7f;
	constexpr float SEAT_BACK_HEIGHT = 1.8f, SEAT_BACK_THICKNESS = 0.25f;
	constexpr float SEAT_DEPTH = 1.5f, SEAT_HEIGHT = 1.3f;

	constexpr glm::vec3 STRIPE_COLOR(1.0f, 0.85f, 0.1f);
	constexpr glm::vec3 WHEEL_COLOR(0.1f, 0.1f, 0.1f);
	constexpr glm::vec3 CAR_COLOR(0.85f, 0.1f, 0.1f);
	constexpr glm::vec3 SEAT_COLOR(0.5f, 0.5f, 0.5f);
}

TrainCar::TrainCar() :
	bodyIndicesStart(0), bodyIndicesCount(0), stripeIndicesStart(0), stripeIndicesCount(0),
	seatIndicesStart(0), seatIndicesCount(0), wheelIndicesStart(0), wheelIndicesCount(0) {
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
	float bodyHeight = CAR_HEIGHT - WHEEL_RADIUS;
	float bodyBottom = WHEEL_RADIUS;
	float halfLen = CAR_LENGTH / 2.0f, halfWidth = CAR_WIDTH / 2.0f;
	float innerHalfLen = halfLen - WALL_THICKNESS, innerHalfWidth = halfWidth - WALL_THICKNESS;

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
	float top = bodyBottom + bodyHeight, innerBottom = bodyBottom + WALL_THICKNESS;

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
	addQuadFace(outer[2], outer[1], outer[5], outer[6], { 1, 0, 0 });
	addQuadFace(outer[0], outer[3], outer[7], outer[4], { -1, 0, 0 });
	addQuadFace(outer[3], outer[2], outer[6], outer[7], { 0, 0, 1 });
	addQuadFace(outer[1], outer[0], outer[4], outer[5], { 0, 0, -1 });

	// Inner walls
	addQuadFace(inner[1], inner[2], inner[6], inner[5], { -1, 0, 0 });
	addQuadFace(inner[3], inner[0], inner[4], inner[7], { 1, 0, 0 });
	addQuadFace(inner[2], inner[3], inner[7], inner[6], { 0, 0, -1 });
	addQuadFace(inner[0], inner[1], inner[5], inner[4], { 0, 0, 1 });

	// Bottom faces
	addQuadFace(outer[0], outer[1], outer[2], outer[3], { 0, -1, 0 });
	addQuadFace(inner[3], inner[2], inner[1], inner[0], { 0, 1, 0 });

	// Top rim
	addQuadFace(outer[5], outer[4], inner[4], inner[5], { 0, 1, 0 });
	addQuadFace(outer[7], outer[6], inner[6], inner[7], { 0, 1, 0 });
	addQuadFace(outer[4], outer[7], inner[7], inner[4], { 0, 1, 0 });
	addQuadFace(outer[6], outer[5], inner[5], inner[6], { 0, 1, 0 });
}

void TrainCar::buildStripe(float bodyBottom, float bodyHeight, float halfLen, float halfWidth, float stripeHeight) {
	float mid = bodyBottom + bodyHeight * 0.5f;
	float bot = mid - stripeHeight / 2.0f, top = mid + stripeHeight / 2.0f;
	float len = halfLen + STRIPE_OFFSET, wid = halfWidth + STRIPE_OFFSET;

	addQuadFace({ len, bot, wid }, { len, bot, -wid }, { len, top, -wid }, { len, top, wid }, { 1, 0, 0 });
	addQuadFace({ -len, bot, -wid }, { -len, bot, wid }, { -len, top, wid }, { -len, top, -wid }, { -1, 0, 0 });
	addQuadFace({ -len, bot, wid }, { len, bot, wid }, { len, top, wid }, { -len, top, wid }, { 0, 0, 1 });
	addQuadFace({ len, bot, -wid }, { -len, bot, -wid }, { -len, top, -wid }, { len, top, -wid }, { 0, 0, -1 });
}

void TrainCar::buildSeats(float bodyBottom, float innerHalfLen, float innerHalfWidth) {
	float seatY = bodyBottom + WALL_THICKNESS;
	float seatPositions[] = { innerHalfLen * FRONT_SEAT_POS_RATIO, innerHalfLen * BACK_SEAT_POS_RATIO };

	for (float xPos : seatPositions) {
		addBox({ xPos - SEAT_DEPTH / 2, seatY, -innerHalfWidth }, { xPos + SEAT_DEPTH / 2, seatY + SEAT_HEIGHT, innerHalfWidth });
		addBox({ xPos - SEAT_DEPTH / 2, seatY + SEAT_HEIGHT, -innerHalfWidth }, { xPos - SEAT_DEPTH / 2 + SEAT_BACK_THICKNESS, seatY + SEAT_HEIGHT + SEAT_BACK_HEIGHT, innerHalfWidth });
	}
}

void TrainCar::addBox(glm::vec3 min, glm::vec3 max) {
	glm::vec3 verts[8] = {
		{min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z},
		{min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}
	};

	addQuadFace(verts[2], verts[1], verts[5], verts[6], { 1, 0, 0 });   // +X face
	addQuadFace(verts[0], verts[3], verts[7], verts[4], { -1, 0, 0 });  // -X face
	addQuadFace(verts[3], verts[2], verts[6], verts[7], { 0, 0, 1 });   // +Z face
	addQuadFace(verts[1], verts[0], verts[4], verts[5], { 0, 0, -1 });  // -Z face
	addQuadFace(verts[4], verts[7], verts[6], verts[5], { 0, 1, 0 });   // +Y face
	addQuadFace(verts[3], verts[0], verts[1], verts[2], { 0, -1, 0 });  // -Y face
}

void TrainCar::buildWheels(float bodyBottom) {
	float lengthOffset = CAR_LENGTH * WHEEL_LENGTH_OFFSET_RATIO;
	float widthOffset = CAR_WIDTH * WHEEL_WIDTH_OFFSET_RATIO;
	float zVariants[] = { -widthOffset / 2.0f, widthOffset / 2.0f };
	float xVariants[] = { lengthOffset, -lengthOffset };

	for (float xPos : xVariants)
		for (float zPos : zVariants)
			addHalfCylinder({ xPos, bodyBottom, zPos }, { 0, 0, 1 }, WHEEL_RADIUS, WHEEL_WIDTH, WHEEL_SIDES);
}

void TrainCar::addHalfCylinder(const glm::vec3& center, const glm::vec3& axis, float radius, float len, int sides) {
	glm::vec3 axisNorm = glm::normalize(axis);
	glm::vec3 start = center - axisNorm * (len * 0.5f);
	glm::vec3 end = center + axisNorm * (len * 0.5f);
	glm::vec3 perp1(1, 0, 0), perp2(0, -1, 0);

	auto ringOffset = [&](int i) {
		float a = i * std::numbers::pi_v<float> / sides;
		return perp1 * std::cos(a) * radius + perp2 * std::sin(a) * radius;
		};

	unsigned baseIdx = vertices.size();
	for (int i = 0; i <= sides; ++i) {
		glm::vec3 off = ringOffset(i);
		glm::vec3 n = glm::normalize(off);
		vertices.push_back({ start + off, n });
		vertices.push_back({ end + off, n });
	}

	for (int i = 0; i < sides; ++i) {
		unsigned b = baseIdx + i * 2;
		indices.insert(indices.end(), { b, b + 1, b + 2, b + 1, b + 3, b + 2 });
	}

	for (int cap = 0; cap < 2; ++cap) {
		glm::vec3 capCenter = cap ? end : start;
		glm::vec3 capNormal = axisNorm * (cap ? 1.0f : -1.0f);
		unsigned capBase = vertices.size();

		vertices.push_back({ capCenter, capNormal });
		for (int i = 0; i <= sides; ++i)
			vertices.push_back({ capCenter + ringOffset(i), capNormal });

		for (int i = 0; i < sides; ++i) {
			if (cap) indices.insert(indices.end(), std::initializer_list<unsigned>{capBase, capBase + i + 2, capBase + i + 1});
			else indices.insert(indices.end(), std::initializer_list<unsigned>{ capBase, capBase + i + 1, capBase + i + 2 });
		}
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

	struct { const glm::vec3& color; unsigned start, count; } parts[] = {
		{CAR_COLOR, bodyIndicesStart, bodyIndicesCount},
		{STRIPE_COLOR, stripeIndicesStart, stripeIndicesCount},
		{SEAT_COLOR, seatIndicesStart, seatIndicesCount},
		{WHEEL_COLOR, wheelIndicesStart, wheelIndicesCount}
	};

	for (const auto& part : parts) {
		shader.setVec3("baseColor", part.color.r, part.color.g, part.color.b);
		glDrawElements(GL_TRIANGLES, part.count, GL_UNSIGNED_INT, (void*)(part.start * sizeof(unsigned int)));
	}

	glBindVertexArray(0);
}