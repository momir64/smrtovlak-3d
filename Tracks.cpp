#include "Tracks.h"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <cmath>

namespace {
	constexpr float CAR_LENGTH = 6.0f, WHEEL_LENGTH_OFFSET_RATIO = 0.32f;
	constexpr float WHEEL_BASE = CAR_LENGTH * WHEEL_LENGTH_OFFSET_RATIO * 2;

	constexpr float SIZE = 8.0f;
	constexpr float ELLIPSE_A = 6.0f * SIZE, ELLIPSE_B = 3.0f * SIZE;

	constexpr float TRACKS_THICKNESS = 0.36f, TRACKS_HALF_WIDTH = 1.5f;
	const Color TRACKS_COLOR(0.8f, 0.4f, 0.1f);
	constexpr float SCALE_HEIGHT = 8.0f * SIZE;

	constexpr float SUPPORT_RADIUS = 0.1f, SUPPORT_SPACING = 1.5f;
	constexpr float SUPPORT_VERTICAL_SPACING = 1.8f;
	const Color SUPPORT_COLOR(0.5f, 0.5f, 0.5f);
	constexpr int SUPPORT_NUM_SIDES = 16;
}

Tracks::Tracks(const std::string& filePath) {
	buildMesh(filePath);

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

void Tracks::draw(const Shader& shader) const {
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", glm::value_ptr(model));
	shader.setBool("useTexture", false);

	shader.setVec3("baseColor", TRACKS_COLOR.red, TRACKS_COLOR.green, TRACKS_COLOR.blue);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, tracksIndicesCount, GL_UNSIGNED_INT, 0);

	shader.setVec3("baseColor", SUPPORT_COLOR.red, SUPPORT_COLOR.green, SUPPORT_COLOR.blue);
	auto supportStart = (void*)(tracksIndicesCount * sizeof(unsigned int));
	glDrawElements(GL_TRIANGLES, indices.size() - tracksIndicesCount, GL_UNSIGNED_INT, supportStart);

	glBindVertexArray(0);
}

void Tracks::buildMesh(const std::string& filePath) {
	auto points2d = LoadPoints(filePath);
	if (points2d.size() < 2)
		return;

	auto extremes = findEllipseExtremes(points2d);
	computeCenters(points2d, extremes);
	computePerpendiculars();
	buildSegmentGeometry();
	buildSupport();
}

std::vector<std::pair<float, float>> Tracks::LoadPoints(const std::string& filePath) {
	std::vector<std::pair<float, float>> points;
	std::ifstream file(filePath);

	if (!file) {
		std::cerr << "Failed to open track file: " << filePath << std::endl;
		return points;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		float x, y;
		if (ss >> x >> y)
			points.emplace_back(x, y);
	}

	file.close();

	return points;
}

Tracks::ElipseExtremes Tracks::findEllipseExtremes(const std::vector<std::pair<float, float>>& points2d) {
	Tracks::ElipseExtremes extremes;
	for (size_t i = 1; i < points2d.size(); ++i) {
		if (points2d[i].first < points2d[extremes.leftEndIdx].first) extremes.leftEndIdx = i;
		if (points2d[i].first > points2d[extremes.rightEndIdx].first) extremes.rightEndIdx = i;
		if (points2d[i].second < extremes.minY || extremes.minY < 0) extremes.minY = points2d[i].second;
	}
	return extremes;
}

void Tracks::computeCenters(const std::vector<std::pair<float, float>>& points2d, Tracks::ElipseExtremes extremes) {
	size_t n = points2d.size();
	points.resize(n);

	for (size_t i = 0; i < n; ++i) {
		float y = (points2d[i].second - extremes.minY) * SCALE_HEIGHT + TRACKS_THICKNESS;
		float dx = points2d[extremes.rightEndIdx].first - points2d[extremes.leftEndIdx].first;
		float x = (points2d[i].first - points2d[extremes.leftEndIdx].first) / dx * 2.0f * ELLIPSE_A - ELLIPSE_A;
		float z = ELLIPSE_B * sqrt(std::max(0.0f, 1.0f - (x * x) / (ELLIPSE_A * ELLIPSE_A)));
		if (i > extremes.leftEndIdx && i < extremes.rightEndIdx) z = -z;
		points[i].center = glm::vec3(x, y, z);
	}
}

void Tracks::computePerpendiculars() {
	const size_t n = points.size();
	if (n < 2) return;

	float totalDist = 0.0f;
	glm::vec3 lastPerp(0.0f, 0.0f, 1.0f);
	for (size_t i = 0; i < n; ++i) {
		points[i].distance = totalDist;
		totalDist += glm::length(points[(i + 1) % n].center - points[i].center);

		glm::vec3 dir = points[(i + 1) % n].center - points[i].center;
		dir.y = 0.0f;
		if (glm::length(dir) > 0.0001f) {
			dir = glm::normalize(dir);
			lastPerp = glm::vec3(-dir.z, 0.0f, dir.x);
		}
		points[i].perp = lastPerp;
	}

	for (size_t i = 0; i < n; ++i) {
		float frontDist = points[i].distance + WHEEL_BASE * 0.5f;
		float backDist = points[i].distance - WHEEL_BASE * 0.5f;
		if (frontDist >= totalDist) frontDist -= totalDist;
		if (backDist < 0) backDist += totalDist;

		size_t frontIdx = 0, backIdx = 0;
		for (size_t j = 0; j < n; ++j) {
			if (points[j].distance <= frontDist) frontIdx = j;
			if (points[j].distance <= backDist)  backIdx = j;
		}

		glm::vec3 d = points[frontIdx].center - points[backIdx].center;
		points[i].pitch = std::atan2(d.y, glm::length(glm::vec2(d.x, d.z)));
	}

	const float alpha = 0.05f;
	float prev = points[n - 1].pitch;
	for (size_t i = 0; i < n; ++i) {
		points[i].pitch = glm::mix(prev, points[i].pitch, alpha);
		prev = points[i].pitch;
	}
}

void Tracks::addQuadFace(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n, bool flip) {
	unsigned int b = vertices.size();
	vertices.insert(vertices.end(), { {v0,n}, {v1,n}, {v2,n}, {v3,n} });
	if (flip) indices.insert(indices.end(), { b, b + 1, b + 2, b + 1, b + 3, b + 2 });
	else indices.insert(indices.end(), { b, b + 2, b + 1, b + 1, b + 2, b + 3 });
}

void Tracks::buildSegmentGeometry() {
	size_t n = points.size();
	if (n < 2) return;

	glm::vec3 prevSurfaceNormal(0.0f, 1.0f, 0.0f);

	for (size_t i = 0; i < n; ++i) {
		size_t next = (i + 1) % n;
		glm::vec3 c0 = points[i].center, c1 = points[next].center, p0 = points[i].perp, p1 = points[next].perp;

		glm::vec3 forward0 = glm::normalize(c1 - c0);
		glm::vec3 forward1 = (i + 2 < n) ? glm::normalize(points[(i + 2) % n].center - c1) : forward0;

		glm::vec3 surfaceNormal0 = glm::normalize(glm::cross(p0, forward0));
		if (glm::dot(surfaceNormal0, prevSurfaceNormal) < 0.0f) surfaceNormal0 = -surfaceNormal0;
		prevSurfaceNormal = surfaceNormal0;

		glm::vec3 surfaceNormal1 = glm::normalize(glm::cross(p1, forward1));
		if (glm::dot(surfaceNormal1, surfaceNormal0) < 0.0f) surfaceNormal1 = -surfaceNormal1;

		glm::vec3 tl0 = c0 + p0 * TRACKS_HALF_WIDTH, tr0 = c0 - p0 * TRACKS_HALF_WIDTH;
		glm::vec3 tl1 = c1 + p1 * TRACKS_HALF_WIDTH, tr1 = c1 - p1 * TRACKS_HALF_WIDTH;

		glm::vec3 bl0 = tl0 - surfaceNormal0 * TRACKS_THICKNESS, br0 = tr0 - surfaceNormal0 * TRACKS_THICKNESS;
		glm::vec3 bl1 = tl1 - surfaceNormal1 * TRACKS_THICKNESS, br1 = tr1 - surfaceNormal1 * TRACKS_THICKNESS;

		glm::vec3 leftNorm = glm::normalize(p0 + p1);

		addQuadFace(tl0, tr0, tl1, tr1, surfaceNormal0, false);
		addQuadFace(bl0, br0, bl1, br1, -surfaceNormal0, true);
		addQuadFace(tl0, bl0, tl1, bl1, leftNorm, false);
		addQuadFace(tr0, br0, tr1, br1, -leftNorm, true);
	}

	tracksIndicesCount = indices.size();
}

void Tracks::buildSupport() {
	if (points.size() < 2) return;
	std::vector<std::pair<glm::vec3, glm::vec3>> braces;

	float length = SUPPORT_SPACING, minY = -1.0f;
	for (size_t i = 0; i <= points.size(); ++i) {
		size_t ix = i % points.size();
		glm::vec3 start = points[ix].center, end = points[(i + 1) % points.size()].center;
		length += glm::distance(glm::vec2(start.x, start.z), glm::vec2(end.x, end.z));
		if (ix > 0) points[i].distance = points[i - 1].distance + glm::distance(start, end);

		if (minY < 0 || start.y < minY)
			minY = start.y;

		if (length > SUPPORT_SPACING) {
			start.y -= TRACKS_THICKNESS * 0.75f;
			addSupportColumn(start, glm::normalize(points[ix].perp), braces, minY);
			length = 0.0f;
			minY = -1.0f;
		}
	}
}

void Tracks::addSupportColumn(const glm::vec3& topCenter, const glm::vec3& perp, std::vector<std::pair<glm::vec3, glm::vec3>>& previousBraces, float minY) {
	glm::vec3 rightTop = topCenter - perp * (TRACKS_HALF_WIDTH - SUPPORT_RADIUS * 1.1f);
	glm::vec3 leftTop = topCenter + perp * (TRACKS_HALF_WIDTH - SUPPORT_RADIUS * 1.1f);
	glm::vec3 rightBottom = glm::vec3(rightTop.x, 0.0f, rightTop.z);
	glm::vec3 leftBottom = glm::vec3(leftTop.x, 0.0f, leftTop.z);
	addCylinder(rightBottom, rightTop, SUPPORT_RADIUS);
	addCylinder(leftBottom, leftTop, SUPPORT_RADIUS);

	float braceHeight = std::max(leftTop.y - SUPPORT_VERTICAL_SPACING * 0.67f, leftTop.y / 2);
	std::vector<std::pair<glm::vec3, glm::vec3>> braces;

	while (braceHeight >= SUPPORT_VERTICAL_SPACING * 0.1f) {
		glm::vec3 rightBrace = glm::vec3(rightTop.x, braceHeight, rightTop.z);
		glm::vec3 leftBrace = glm::vec3(leftTop.x, braceHeight, leftTop.z);
		addCylinder(leftBrace, rightBrace, SUPPORT_RADIUS * 0.64f);
		braces.push_back(std::pair(leftBrace, rightBrace));

		for (auto& braces : previousBraces) {
			if (braces.first.y < minY && braces.first.y < braceHeight + SUPPORT_VERTICAL_SPACING && braces.first.y >= braceHeight - SUPPORT_VERTICAL_SPACING) {
				addCylinder(braces.first, leftBrace, SUPPORT_RADIUS * 0.64f);
				addCylinder(braces.second, rightBrace, SUPPORT_RADIUS * 0.64f);
			}
		}

		braceHeight -= SUPPORT_VERTICAL_SPACING;
	}

	previousBraces = braces;
}

void Tracks::addCylinder(const glm::vec3& bottom, const glm::vec3& top, float radius) {
	glm::vec3 axis = top - bottom;
	float length = glm::length(axis);
	if (length < 0.001f) return;

	glm::vec3 axisNorm = axis / length;
	glm::vec3 arbitrary = (fabs(axisNorm.y) < 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
	glm::vec3 perp1 = glm::normalize(glm::cross(axisNorm, arbitrary));
	glm::vec3 perp2 = glm::cross(axisNorm, perp1);

	unsigned baseIdx = vertices.size();
	for (int i = 0; i <= SUPPORT_NUM_SIDES; ++i) {
		float angle = i * 2.0f * std::numbers::pi_v<float> / SUPPORT_NUM_SIDES;
		glm::vec3 offset = perp1 * cos(angle) * radius + perp2 * sin(angle) * radius;
		glm::vec3 normal = glm::normalize(offset);
		vertices.push_back({ bottom + offset, normal });
		vertices.push_back({ top + offset, normal });
	}

	for (int i = 0; i < SUPPORT_NUM_SIDES; ++i) {
		unsigned b0 = baseIdx + i * 2, b1 = b0 + 1, b2 = baseIdx + (i + 1) * 2, b3 = b2 + 1;
		indices.insert(indices.end(), { b0, b2, b1, b1, b2, b3 });
	}
}
