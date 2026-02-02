#include "DataClasses.h"

Color::Color(float red, float green, float blue) :
	red(red), green(green), blue(blue) {
}

Color::Color(int red, int green, int blue) :
	red(float(red) / 255.f), green(float(green) / 255.f), blue(float(blue) / 255.f) {
}

Bounds::Bounds(float x, float y, float width, float height, float angle, bool flip) :
	x(x), y(y), width(width), height(height), angle(angle), flip(flip) {
}