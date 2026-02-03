#include "Character.h"

namespace {
	constexpr float CHARACTER_SCALE = 3.0f, CHARACTER_BRIGHTNESS = 2.0f;

	constexpr float BELT_Y_OFFSET = -1.15f, BELT_RIGHT_OFFSET = 0.3f, BELT_FORWARD_OFFSET = 0.0f;

	constexpr float CAR_LENGTH = 6.0f, WHEEL_RADIUS = 0.4f, WALL_THICKNESS = 0.08f, SEAT_HEIGHT = 1.3f;
	constexpr float CHARACTER_Y_OFFSET = WHEEL_RADIUS + WALL_THICKNESS + SEAT_HEIGHT * 0.1f;

	constexpr float FRONT_SEAT_POS_RATIO = 0.3f, BACK_SEAT_POS_RATIO = -0.7f;
	constexpr float CHARACTER_BACKWARD_OFFSET = -0.5f;

	constexpr float CHARACTER_FORWARD_OFFSET_FRONT = (CAR_LENGTH / 2.0f - WALL_THICKNESS) * FRONT_SEAT_POS_RATIO + CHARACTER_BACKWARD_OFFSET;
	constexpr float CHARACTER_FORWARD_OFFSET_BACK = (CAR_LENGTH / 2.0f - WALL_THICKNESS) * BACK_SEAT_POS_RATIO + CHARACTER_BACKWARD_OFFSET;
}

Character::Character(const Model& belt, const std::string& modelPath, bool frontSeat, bool showBelt) :
	belt(belt), showBelt(showBelt), frontSeat(frontSeat), model(Model(modelPath, CHARACTER_SCALE, CHARACTER_BRIGHTNESS)) {
}

void Character::draw(const Shader& shader, const glm::vec3& carPosition, const glm::vec3& carForward, const glm::vec3& carUp) const {
	float forwardOffset = frontSeat ? CHARACTER_FORWARD_OFFSET_FRONT : CHARACTER_FORWARD_OFFSET_BACK;

	glm::vec3 worldPosition = carPosition + carForward * forwardOffset + carUp * CHARACTER_Y_OFFSET;
	glm::vec3 characterForward = -carForward;
	glm::vec3 characterUp = carUp;

	model.draw(shader, worldPosition, characterForward, characterUp);

	if (showBelt) {
		glm::vec3 characterRight = glm::normalize(glm::cross(characterForward, characterUp));
		glm::vec3 beltPosition = worldPosition + characterUp * BELT_Y_OFFSET + characterRight * BELT_RIGHT_OFFSET + characterForward * BELT_FORWARD_OFFSET;
		belt.draw(shader, beltPosition, characterForward, characterUp);
	}
}