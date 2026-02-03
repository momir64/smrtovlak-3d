#pragma once
#include "Shader.h"
#include "Model.h"
#include <string>

class Character {
	const Model& belt;
	Model model;

	bool frontSeat;

public:
	bool showBelt;

	Character(const Model& belt, const std::string& modelPath, bool frontSeat, bool showBelt = false);
	~Character() = default;

	Character(Character&&) = default;
	Character& operator=(Character&&) = default;

	void draw(const Shader& shader, const glm::vec3& carPosition, const glm::vec3& carForward, const glm::vec3& carUp) const;
};