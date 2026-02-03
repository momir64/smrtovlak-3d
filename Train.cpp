#include "Train.h"
#include <cmath>

namespace {
	constexpr float CAMERA_FORWARD_OFFSET = 2.0f;
	constexpr float CAMERA_HEIGHT_OFFSET = 6.0f;
}

Train::Train(const Tracks& tracks, int carCount)
	: tracks(tracks), carCount(carCount), offset(-2.4f) {
	if (tracks.points.size() < 2) {
		spacing = 0.0f;
		return;
	}
	float totalLength = tracks.points.back().distance;
	spacing = 9.0f;
}

void Train::update(float delta) {
	if (tracks.points.empty()) return;
	offset += delta * 32.0f;
	float totalLength = tracks.points.back().distance;
	if (offset >= totalLength) offset = std::fmod(offset, totalLength);
}

void Train::draw(const Shader& shader) const {
	if (tracks.points.empty()) return;

	float totalLength = tracks.points.back().distance;
	size_t n = tracks.points.size();

	for (int i = 0; i < carCount; ++i) {
		float targetDist = offset - i * spacing;
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
	}
}

OrientedPoint Train::getFrontCarTransform() const {
	OrientedPoint transform{ glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	if (tracks.points.empty()) return transform;

	float totalLength = tracks.points.back().distance;
	size_t n = tracks.points.size();

	float targetDist = offset;
	while (targetDist < 0.0f) targetDist += totalLength;
	while (targetDist >= totalLength) targetDist -= totalLength;

	size_t idx = 0;
	for (size_t j = 0; j < n; ++j) {
		if (tracks.points[j].distance <= targetDist)
			idx = j;
		else
			break;
	}

	const auto& p = tracks.points[idx];

	glm::vec3 right = glm::normalize(p.perp);
	glm::vec3 forward = -glm::normalize(glm::vec3(-right.z * cos(-p.pitch), sin(-p.pitch), right.x * cos(-p.pitch)));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	transform.position = p.center + forward * CAMERA_FORWARD_OFFSET + up * CAMERA_HEIGHT_OFFSET;
	transform.forward = forward;
	transform.up = up;

	return transform;
}