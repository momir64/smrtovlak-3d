#pragma once
#include <unordered_map>
#include "DataClasses.h"
#include <glm/glm.hpp>
#include "Shader.h"
#include <string>
#include <vector>

struct Material {
	std::string name;
	glm::vec3 ambient = glm::vec3(0.2f);
	glm::vec3 diffuse = glm::vec3(0.8f);
	glm::vec3 specular = glm::vec3(0.0f);
	float shininess = 32.0f;
};

struct MeshGroup {
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
	unsigned int indexCount = 0;
	Material material;
};

class Model {
	std::vector<MeshGroup> meshGroups;
	std::string directory;

	void loadOBJ(const std::string& path);
	std::unordered_map<std::string, Material> loadMTL(const std::string& path);

public:
	float brightness;
	float scale;

	Model(const std::string& objPath, float scale = 1.0f, float brightness = 1.0f);
	~Model();

	Model(Model&& other) noexcept;
	Model& operator=(Model&& other) noexcept;

	void draw(const Shader& shader, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up) const;
};