#pragma once
#include "Tracks.h"
#include "TrainCar.h"
#include "Shader.h"

class Train {
	const Tracks& tracks;
	TrainCar car;
	int carCount;
	float spacing;
	float offset;

public:
	Train(const Tracks& tracks, int carCount = 4);

	void update(float delta);
	void draw(const Shader& shader) const;

	OrientedPoint getFrontCarTransform() const;
};