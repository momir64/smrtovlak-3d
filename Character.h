#pragma once
#include "Shader.h"
#include "Model.h"
#include <string>

class Character {
	const Model& belt;
	Model model;

public:
	bool showBelt = false;
	bool visible = true;
	bool sick = false;
	bool frontSeat;

	Character(const Model& belt, const std::string& modelPath, bool frontSeat);
	~Character() = default;

	Character(Character&&) = default;
	Character& operator=(Character&&) = default;
	friend void swap(Character& a, Character& b) noexcept;

	void draw(const Shader& shader, const glm::vec3& carPosition, const glm::vec3& carForward, const glm::vec3& carUp) const;
};

inline void swap(Character& a, Character& b) noexcept {
	using std::swap;
	swap(a.model, b.model);
	swap(a.frontSeat, b.frontSeat);
	swap(a.showBelt, b.showBelt);
	swap(a.visible, b.visible);
	swap(a.sick, b.sick);
}