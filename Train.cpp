#include "Train.h"
#include <cmath>

namespace {
	constexpr float TRAIN_START_OFFSET = -3.8f, TRAIN_CAR_SPACE = 9.0f;
	constexpr float TRAIN_SPEED = 0.0f;
	constexpr int TRAIN_CAR_COUNT = 4;

	constexpr float CAMERA_FORWARD_OFFSET = 1.0f, CAMERA_HEIGHT_OFFSET = 5.0f;

	const std::string BELT_MODEL_PATH = "assets/models/belt.obj";
	constexpr float BELT_SCALE = 3.0f, BELT_BRIGHTNESS = 2.0f;

	const std::vector<std::string> CHARACTER_MODELS = {
		"assets/models/m_casual.obj",
		"assets/models/m_farmer.obj",
		"assets/models/m_punk.obj",
		"assets/models/m_suit.obj",
		"assets/models/w_soldier.obj",
		"assets/models/w_worker.obj",
		"assets/models/w_witch.obj",
		"assets/models/w_punk.obj"
	};
}

Train::Train(const Tracks& tracks)
	: tracks(tracks), offset(TRAIN_START_OFFSET), belt(Model(BELT_MODEL_PATH, BELT_SCALE, BELT_BRIGHTNESS)) {

	for (int i = 0; i < TRAIN_CAR_COUNT; i++) {
		characters.push_back(Character(belt, CHARACTER_MODELS[i * 2], true));
		characters.push_back(Character(belt, CHARACTER_MODELS[i * 2 + 1], false));
	}
}

OrientedPoint Train::getCarTransform(int carIndex) const {
	OrientedPoint transform{ glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	if (tracks.points.empty()) return transform;

	float totalLength = tracks.points.back().distance;
	size_t n = tracks.points.size(), idx = 0;

	float targetDist = offset - carIndex * TRAIN_CAR_SPACE;
	while (targetDist < 0.0f) targetDist += totalLength;
	while (targetDist >= totalLength) targetDist -= totalLength;
	while (idx < n && tracks.points[idx].distance <= targetDist) ++idx;

	const auto& point = tracks.points[idx ? idx - 1 : 0];
	glm::vec3 right = glm::normalize(point.perp);
	glm::vec3 forward = -glm::normalize(glm::vec3(-right.z * cos(-point.pitch), sin(-point.pitch), right.x * cos(-point.pitch)));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	return { point.center, forward, up };
}

OrientedPoint Train::getFrontCarTransform() const {
	OrientedPoint transform = getCarTransform(0);
	transform.position += transform.forward * CAMERA_FORWARD_OFFSET + transform.up * CAMERA_HEIGHT_OFFSET;
	return transform;
}

void Train::toggleBelt(int seatNumber) {
	seatNumber--;
	if (seatNumber >= 0 && seatNumber < characters.size())
		characters[seatNumber].showBelt = !characters[seatNumber].showBelt;
}

void Train::update(float delta) {
	if (tracks.points.empty()) return;
	offset += delta * TRAIN_SPEED;
	float totalLength = tracks.points.back().distance;
	if (offset >= totalLength) offset = std::fmod(offset, totalLength);
}

void Train::draw(const Shader& shader) const {
	if (tracks.points.empty()) return;

	float totalLength = tracks.points.back().distance;
	size_t n = tracks.points.size();

	for (int i = 0; i < TRAIN_CAR_COUNT; ++i) {
		float targetDist = offset - i * TRAIN_CAR_SPACE;
		while (targetDist < 0.0f) targetDist += totalLength;

		size_t idx = 0;
		for (size_t j = 0; j < n; ++j) {
			if (tracks.points[j].distance <= targetDist)
				idx = j;
			else
				break;
		}

		const auto& p = tracks.points[idx];
		car.draw(shader, p.center, p.perp, p.pitch);

		OrientedPoint carTransform = getCarTransform(i);
		int frontSeatIndex = i * 2, backSeatIndex = i * 2 + 1;
		if (frontSeatIndex < (int)characters.size())
			characters[frontSeatIndex].draw(shader, carTransform.position, carTransform.forward, carTransform.up);
		if (backSeatIndex < (int)characters.size())
			characters[backSeatIndex].draw(shader, carTransform.position, carTransform.forward, carTransform.up);
	}
}