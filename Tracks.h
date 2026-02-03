#pragma once
#include "DataClasses.h"
#include <glm/glm.hpp>
#include "Shader.h"
#include <vector>
#include <string>

class Tracks {
	struct ElipseExtremes {
		size_t leftEndIdx = 0, rightEndIdx = 0;
		float minY = -1;
	};

	unsigned int VAO = 0, VBO = 0, EBO = 0;
	unsigned int tracksIndicesCount = 0;
	std::vector<unsigned int> indices;
	std::vector<Vertex> vertices;

	void buildMesh(const std::string& filePath);
	std::vector<std::pair<float, float>> LoadPoints(const std::string& filePath);
	ElipseExtremes findEllipseExtremes(const std::vector<std::pair<float, float>>& points2d);
	void computeCenters(const std::vector<std::pair<float, float>>& points2d, Tracks::ElipseExtremes extremes);
	void computePerpendiculars();
	void buildSegmentGeometry();
	void addQuadFace(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n);
	void buildSupport();
	void addSupportColumn(const glm::vec3& topCenter, const glm::vec3& perp, std::vector<std::pair<glm::vec3, glm::vec3>>& previousBraces, float minY);
	void addCylinder(const glm::vec3& bottom, const glm::vec3& top, float radius);

public:
	std::vector<TrackPoint> points;

	Tracks(const std::string& filePath);
	void draw(const Shader& shader) const;
};