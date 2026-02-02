#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "Ground.h"
#include <string>


namespace {
	constexpr float SIDE_LENGTH = 100.0f;
	constexpr int TILES_COUNT = 20;
}


static float vertices[] = {
	-SIDE_LENGTH,0,-SIDE_LENGTH, 0,1,0, 0,0,
	 SIDE_LENGTH,0,-SIDE_LENGTH, 0,1,0, TILES_COUNT,0,
	 SIDE_LENGTH,0, SIDE_LENGTH, 0,1,0, TILES_COUNT,TILES_COUNT,
	-SIDE_LENGTH,0, SIDE_LENGTH, 0,1,0, 0,TILES_COUNT
};

static unsigned int indices[] = { 0,1,2, 2,3,0 };

Ground::Ground(const std::string& texturePath) {
	initMesh();
	loadTexture(texturePath);
}

void Ground::initMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Ground::loadTexture(const std::string& path) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height, colors;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &colors, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, colors == 4 ? GL_RGBA : GL_RGB, width, height, 0, colors == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
}

void Ground::draw(const Shader& shader) const {
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", &model[0][0]);
	shader.setBool("useTexture", true);

	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
