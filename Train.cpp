#include "Train.h"
#include <algorithm>
#include <random>
#include <cmath>

namespace {
	constexpr float TRAIN_START_OFFSET = -3.8f, TRAIN_CAR_SPACE = 8.6f;
	constexpr int TRAIN_CAR_COUNT = 4;

	constexpr float TRAIN_MIN_SPEED = 3.8f, TRAIN_MAX_SPEED = 64.0f, TRAIN_MAX_SPEED_SICK = 8.0f;
	constexpr float SLOWDOWN_DISTANCE = 10.0f, FINISH_SLOWDOWN_DISTANCE_SICK = 18.0f;
	constexpr float FINISH_SLOWDOWN_DISTANCE = 42.0f, FINISHED_DISTANCE = 0.05f;
	constexpr float TRAIN_FLAT_ACCEL = 7.0f, TRAIN_SLOPE_FACTOR = 32.0f;

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
	: offset(TRAIN_START_OFFSET), currentSpeed(0.0f), sleepTimer(0.0f), preStopSpeed(0.0f), stopDistance(0.0f),
	tracks(tracks), belt(Model(BELT_MODEL_PATH, BELT_SCALE, BELT_BRIGHTNESS)), charactersCount(0) {

	for (int i = 0; i < TRAIN_CAR_COUNT; i++) {
		characters.push_back(Character(belt, CHARACTER_MODELS[i * 2], true));
		characters.push_back(Character(belt, CHARACTER_MODELS[i * 2 + 1], false));
	}

	shuffleCharacters();
}

OrientedPoint Train::getCarTransform(int carIndex) const {
	OrientedPoint transform{ glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	if (tracks.points.empty()) return transform;

	float totalLength = tracks.points.back().distance;
	size_t n = tracks.points.size(), idx = 0;

	float targetDist = offset - carIndex * TRAIN_CAR_SPACE;
	if (offset < totalLength + TRAIN_START_OFFSET - FINISH_SLOWDOWN_DISTANCE) {
		while (targetDist < 0.0f) targetDist += totalLength;
		while (targetDist >= totalLength) targetDist -= totalLength;
	} else {
		if (targetDist < 0.0f) targetDist = 0.0f;
		if (targetDist >= totalLength) targetDist = totalLength - 0.01f;
	}

	while (idx < n && tracks.points[idx].distance <= targetDist) ++idx;

	const auto& point = tracks.points[idx ? idx - 1 : 0];
	glm::vec3 right = glm::normalize(point.perp);
	glm::vec3 forward = -glm::normalize(glm::vec3(-right.z * cos(-point.pitch), sin(-point.pitch), right.x * cos(-point.pitch)));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	return { point.center, forward, up };
}

OrientedPoint Train::getCameraTransform() const {
	OrientedPoint transform = getCarTransform(0);
	transform.position += transform.forward * CAMERA_FORWARD_OFFSET + transform.up * CAMERA_HEIGHT_OFFSET;
	return transform;
}

void Train::shuffleCharacters() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::shuffle(characters.begin(), characters.end(), gen);
	for (int i = 0; i < characters.size(); i++)
		characters[i].frontSeat = (i % 2) == 0;
}

void Train::buckleUp(int seatNumber) {
	if (seatNumber >= 0 && seatNumber < charactersCount)
		characters[seatNumber].showBelt = true;
}

TrainMode Train::getMode() const {
	return mode;
}

void Train::setMode(TrainMode newMode) {
	mode = newMode;
}

void Train::addCharacter() {
	if (charactersCount < characters.size()) {
		characters[charactersCount].visible = true;
		charactersCount++;
	}
}

int Train::getCharactersCount() const {
	return charactersCount;
}

void Train::start() {
	if (charactersCount == 0) return;
	for (int i = 0; i < charactersCount; i++)
		if (!characters[i].showBelt)
			return;
	mode = TrainMode::RUNNING;
}

void Train::makeSick(int seatNumber) {
	if (seatNumber < 0 || seatNumber >= charactersCount) return;
	stopDistance = std::min(offset + SLOWDOWN_DISTANCE, tracks.points.back().distance + TRAIN_START_OFFSET);
	characters[seatNumber].sick = true;
	mode = TrainMode::EMERGENCY_STOP;
	preStopSpeed = currentSpeed;
}

void Train::reset() {
	offset = TRAIN_START_OFFSET;
	mode = TrainMode::FINISHED;
	currentSpeed = 0.0f;
	preStopSpeed = 0.0f;
	stopDistance = 0.0f;
	sleepTimer = 0.0f;
	charactersCount = 0;

	for (auto& character : characters) {
		character.showBelt = false;
		character.visible = false;
		character.sick = false;
	}

	shuffleCharacters();
}

void Train::update(float delta) {
	if (tracks.points.empty()) return;
	if (delta > 0.5f) delta = 0.016f;

	if (sleepTimer > 0.0f) {
		sleepTimer -= delta;
		return;
	}

	float totalLength = tracks.points.back().distance;

	if (mode == TrainMode::RUNNING) {
		std::vector<float> carSpeeds(TRAIN_CAR_COUNT, currentSpeed);

		for (int i = 0; i < TRAIN_CAR_COUNT; i++) {
			float targetDist = offset - i * TRAIN_CAR_SPACE;
			while (targetDist < 0.0f) targetDist += totalLength;
			while (targetDist >= totalLength) targetDist -= totalLength;

			size_t idx = 0;
			while (idx < tracks.points.size() && tracks.points[idx].distance <= targetDist) ++idx;
			const auto& point = tracks.points[idx ? idx - 1 : 0];

			float slope = -std::sin(point.pitch);
			float accel = TRAIN_FLAT_ACCEL + slope * TRAIN_SLOPE_FACTOR;
			if (carSpeeds[i] < TRAIN_MIN_SPEED) accel = TRAIN_SLOPE_FACTOR;

			carSpeeds[i] += accel * delta;
			carSpeeds[i] = std::clamp(carSpeeds[i], 0.0f, TRAIN_MAX_SPEED);

			float endDist = totalLength + TRAIN_START_OFFSET;
			float remaining = endDist - offset;

			if (remaining <= FINISH_SLOWDOWN_DISTANCE) {
				float t = std::clamp(remaining / FINISH_SLOWDOWN_DISTANCE, 0.0f, 1.0f);
				carSpeeds[i] = preStopSpeed * std::pow(t, 0.82f);

				if (remaining <= FINISHED_DISTANCE) {
					carSpeeds[i] = 0.0f;
					offset = endDist;
					reset();
					return;
				}
			} else {
				preStopSpeed = currentSpeed;
			}
		}

		if (!carSpeeds.empty()) {
			float weightedSum = 0.0f;
			float totalWeight = 0.0f;
			int count = carSpeeds.size();

			for (int i = 0; i < count; i++) {
				float weight = float(count - i);
				weightedSum += carSpeeds[i] * weight;
				totalWeight += weight;
			}

			currentSpeed = weightedSum / totalWeight;
			offset += currentSpeed * delta;
		}
	} else if (mode == TrainMode::EMERGENCY_STOP) {
		float remaining = stopDistance - offset;
		float t = std::clamp(remaining / SLOWDOWN_DISTANCE, 0.0f, 1.0f);
		currentSpeed = preStopSpeed * std::pow(t, 0.82f);

		if (remaining <= FINISHED_DISTANCE) {
			offset = stopDistance;
			currentSpeed = 0.0f;
			sleepTimer = 10.0f;
			mode = TrainMode::SICK_MODE;
		}

		offset += currentSpeed * delta;
	} else if (mode == TrainMode::SICK_MODE) {
		float accel = TRAIN_FLAT_ACCEL;
		currentSpeed += accel * delta;
		currentSpeed = std::clamp(currentSpeed, 0.0f, TRAIN_MAX_SPEED_SICK);

		float endDist = totalLength + TRAIN_START_OFFSET;
		float remaining = endDist - offset;

		if (remaining <= FINISH_SLOWDOWN_DISTANCE_SICK) {
			float t = std::clamp(remaining / FINISH_SLOWDOWN_DISTANCE_SICK, 0.0f, 1.0f);
			currentSpeed = preStopSpeed * std::pow(t, 0.7f);

			if (remaining <= FINISHED_DISTANCE) {
				currentSpeed = 0.0f;
				offset = endDist;
				reset();
				return;
			}
		} else {
			preStopSpeed = currentSpeed;
		}

		offset += currentSpeed * delta;
	}
}

void Train::draw(const Shader& shader, bool cameraInTrain) const {
	if (tracks.points.empty()) return;

	float totalLength = tracks.points.back().distance;
	size_t n = tracks.points.size();

	for (int i = 0; i < TRAIN_CAR_COUNT; ++i) {
		float targetDist = offset - i * TRAIN_CAR_SPACE;

		if (offset < totalLength + TRAIN_START_OFFSET - FINISH_SLOWDOWN_DISTANCE)
			while (targetDist < 0.0f) targetDist += totalLength;
		else
			targetDist = std::clamp(targetDist, 0.0f, totalLength - 0.01f);

		size_t idx = 0;
		while (idx + 1 < n && tracks.points[idx + 1].distance <= targetDist) idx++;

		const auto& p = tracks.points[idx];
		car.draw(shader, p.center, p.perp, p.pitch);

		OrientedPoint carTransform = getCarTransform(i);
		int frontSeatIndex = i * 2, backSeatIndex = i * 2 + 1;
		if (frontSeatIndex < (int)characters.size())
			characters[frontSeatIndex].draw(shader, carTransform.position, carTransform.forward, carTransform.up, frontSeatIndex == 0 && cameraInTrain);
		if (backSeatIndex < (int)characters.size())
			characters[backSeatIndex].draw(shader, carTransform.position, carTransform.forward, carTransform.up);
	}
}