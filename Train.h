#pragma once
#include "Character.h"
#include "TrainCar.h"
#include "Tracks.h"
#include "Shader.h"
#include <vector>

class Train {
	std::vector<Character> characters;
	const Tracks& tracks;
	float offset;
	TrainCar car;
	Model belt;

	OrientedPoint getCarTransform(int carIndex) const;

public:
	Train(const Tracks& tracks);

	void update(float delta);
	void draw(const Shader& shader) const;

	OrientedPoint getFrontCarTransform() const;
	void toggleBelt(int seatNumber);
};