#pragma once
#include "Character.h"
#include "TrainCar.h"
#include "Tracks.h"
#include "Shader.h"
#include <vector>

enum class TrainMode {
	WAITING,
	RUNNING,
	EMERGENCY_STOP,
	SICK_MODE,
	FINISHED
};

class Train {
	float offset, currentSpeed, preStopSpeed, stopDistance;
	TrainMode mode = TrainMode::WAITING;
	std::vector<Character> characters;
	const Tracks& tracks;
	int charactersCount;
	float sleepTimer;
	TrainCar car;
	Model belt;

	OrientedPoint getCarTransform(int carIndex) const;

public:
	Train(const Tracks& tracks);

	void draw(const Shader& shader, bool cameraInTrain) const;
	void update(float delta);

	OrientedPoint getCameraTransform() const;
	void buckleUp(int seatNumber);
	void shuffleCharacters();

	TrainMode getMode() const;
	void setMode(TrainMode newMode);
	void makeSick(int seatNumber);
	int getCharactersCount() const;
	void addCharacter();
	void start();
	void reset();
};