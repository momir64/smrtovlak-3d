#include "Model.h"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

Model::Model(const std::string& objPath, float scale, float brightness) : scale(scale), brightness(brightness) {
	size_t lastSlash = objPath.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ? objPath.substr(0, lastSlash + 1) : "";
	loadOBJ(objPath);
}

Model::~Model() {
	for (auto& group : meshGroups) {
		if (group.VAO) glDeleteVertexArrays(1, &group.VAO);
		if (group.VBO) glDeleteBuffers(1, &group.VBO);
		if (group.EBO) glDeleteBuffers(1, &group.EBO);
	}
}

Model::Model(Model&& other) noexcept : meshGroups(std::move(other.meshGroups)), directory(std::move(other.directory)), scale(other.scale), brightness(other.brightness) {
	for (auto& group : other.meshGroups) {
		group.VAO = 0;
		group.VBO = 0;
		group.EBO = 0;
	}
}

Model& Model::operator=(Model&& other) noexcept {
	if (this != &other) {
		for (auto& group : meshGroups) {
			if (group.VAO) glDeleteVertexArrays(1, &group.VAO);
			if (group.VBO) glDeleteBuffers(1, &group.VBO);
			if (group.EBO) glDeleteBuffers(1, &group.EBO);
		}

		meshGroups = std::move(other.meshGroups);
		directory = std::move(other.directory);
		brightness = other.brightness;
		scale = other.scale;

		for (auto& group : other.meshGroups) {
			group.VAO = 0;
			group.VBO = 0;
			group.EBO = 0;
		}
	}
	return *this;
}

std::unordered_map<std::string, Material> Model::loadMTL(const std::string& path) {
	std::unordered_map<std::string, Material> materials;
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Failed to open MTL file: " << path << std::endl;
		return materials;
	}

	Material* currentMaterial = nullptr;
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "newmtl") {
			std::string name;
			iss >> name;
			currentMaterial = &(materials[name] = { name });
		} else if (currentMaterial) {
			if (prefix == "Ka") iss >> currentMaterial->ambient.r >> currentMaterial->ambient.g >> currentMaterial->ambient.b;
			else if (prefix == "Kd") iss >> currentMaterial->diffuse.r >> currentMaterial->diffuse.g >> currentMaterial->diffuse.b;
			else if (prefix == "Ks") iss >> currentMaterial->specular.r >> currentMaterial->specular.g >> currentMaterial->specular.b;
			else if (prefix == "Ns") iss >> currentMaterial->shininess;
		}
	}
	return materials;
}

void Model::loadOBJ(const std::string& path) {
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Failed to open OBJ file: " << path << std::endl;
		return;
	}

	std::vector<glm::vec3> positions, normals;
	std::unordered_map<std::string, std::vector<Vertex>> groupVertices;
	std::unordered_map<std::string, std::vector<unsigned int>> groupIndices;
	std::unordered_map<std::string, Material> materials = { {"default", {}} };
	std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>> groupVertexCache;
	std::string currentMaterial = "default";

	auto parseVertex = [&](const std::string& token) {
		Vertex vertex{};
		size_t firstSlash = token.find('/');
		size_t secondSlash = token.find('/', firstSlash + 1);

		int posIdx = std::stoi(token.substr(0, firstSlash));
		int normIdx = (secondSlash != std::string::npos && secondSlash + 1 < token.size())
			? std::stoi(token.substr(secondSlash + 1)) : 0;

		if (posIdx < 0) posIdx = positions.size() + posIdx + 1;
		if (normIdx < 0) normIdx = normals.size() + normIdx + 1;

		if (posIdx > 0 && posIdx <= (int)positions.size()) vertex.position = positions[posIdx - 1];
		if (normIdx > 0 && normIdx <= (int)normals.size()) vertex.normal = normals[normIdx - 1];
		return vertex;
		};

	auto getOrCreateVertex = [&](const std::string& token) {
		auto& cache = groupVertexCache[currentMaterial];
		auto it = cache.find(token);
		if (it != cache.end()) return it->second;

		unsigned int index = groupVertices[currentMaterial].size();
		groupVertices[currentMaterial].push_back(parseVertex(token));
		cache[token] = index;
		return index;
		};

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "mtllib") {
			std::string mtlFile;
			iss >> mtlFile;
			auto loaded = loadMTL(directory + mtlFile);
			materials.insert(loaded.begin(), loaded.end());
		} else if (prefix == "v") {
			glm::vec3 pos;
			iss >> pos.x >> pos.y >> pos.z;
			positions.push_back(pos);
		} else if (prefix == "vn") {
			glm::vec3 norm;
			iss >> norm.x >> norm.y >> norm.z;
			normals.push_back(norm);
		} else if (prefix == "usemtl") {
			iss >> currentMaterial;
			if (materials.find(currentMaterial) == materials.end())
				materials[currentMaterial] = { currentMaterial };
		} else if (prefix == "f") {
			std::vector<unsigned int> indices;
			std::string token;
			while (iss >> token) indices.push_back(getOrCreateVertex(token));

			for (size_t i = 1; i + 1 < indices.size(); ++i) {
				groupIndices[currentMaterial].push_back(indices[0]);
				groupIndices[currentMaterial].push_back(indices[i]);
				groupIndices[currentMaterial].push_back(indices[i + 1]);
			}
		}
	}

	for (auto& [materialName, vertices] : groupVertices) {
		if (vertices.empty()) continue;

		bool hasNormals = std::any_of(vertices.begin(), vertices.end(),
			[](const auto& v) { return glm::length(v.normal) > 0.001f; });

		if (!hasNormals) {
			auto& idxList = groupIndices[materialName];
			std::vector<glm::vec3> accumulatedNormals(vertices.size(), glm::vec3(0.0f));

			for (size_t i = 0; i + 2 < idxList.size(); i += 3) {
				glm::vec3 faceNormal = glm::cross(
					vertices[idxList[i + 1]].position - vertices[idxList[i]].position,
					vertices[idxList[i + 2]].position - vertices[idxList[i]].position);
				accumulatedNormals[idxList[i]] += faceNormal;
				accumulatedNormals[idxList[i + 1]] += faceNormal;
				accumulatedNormals[idxList[i + 2]] += faceNormal;
			}

			for (size_t i = 0; i < vertices.size(); ++i) {
				vertices[i].normal = glm::length(accumulatedNormals[i]) > 0.001f
					? glm::normalize(accumulatedNormals[i]) : glm::vec3(0.0f, 1.0f, 0.0f);
			}
		}

		MeshGroup group;
		group.material = materials[materialName];
		group.indexCount = groupIndices[materialName].size();

		glGenVertexArrays(1, &group.VAO);
		glGenBuffers(1, &group.VBO);
		glGenBuffers(1, &group.EBO);
		glBindVertexArray(group.VAO);

		glBindBuffer(GL_ARRAY_BUFFER, group.VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, groupIndices[materialName].size() * sizeof(unsigned int),
			groupIndices[materialName].data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);

		meshGroups.push_back(std::move(group));
	}
}

void Model::draw(const Shader& shader, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up) const {
	glm::vec3 f = glm::normalize(forward), u = glm::normalize(up);
	glm::vec3 r = glm::normalize(glm::cross(f, u));
	u = glm::normalize(glm::cross(r, f));

	glm::mat4 model(1.0f);
	model[0] = glm::vec4(r * scale, 0.0f);
	model[1] = glm::vec4(u * scale, 0.0f);
	model[2] = glm::vec4(-f * scale, 0.0f);
	model[3] = glm::vec4(position, 1.0f);

	shader.setMat4("model", glm::value_ptr(model));

	for (const auto& group : meshGroups) {
		glm::vec3 color = group.material.diffuse * brightness;
		shader.setVec3("baseColor", color.r, color.g, color.b);
		glBindVertexArray(group.VAO);
		glDrawElements(GL_TRIANGLES, group.indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}