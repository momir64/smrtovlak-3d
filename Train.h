#pragma once
#include "Character.h"
#include "TrainCar.h"
#include "Tracks.h"
#include "Shader.h"
#include <vector>

enum class TrainMode {
	WAITING = 0,
	RUNNING = 1,
	EMERGENCY_STOP = 2,
	SICK_MODE = 3
};

class Train {
	float offset, currentSpeed, preStopSpeed, stopDistance;
	TrainMode mode = TrainMode::RUNNING;
	std::vector<Character> characters;
	const Tracks& tracks;
	float sleepTimer;
	TrainCar car;
	Model belt;

	OrientedPoint getCarTransform(int carIndex) const;

public:
	Train(const Tracks& tracks);

	void update(float delta);
	void draw(const Shader& shader) const;

	OrientedPoint getFrontCarTransform() const;
	void triggerEmergencyStop(float distance);
	void toggleBelt(int seatNumber);
	void shuffleCharacters();
};