#pragma once
#include <cstdint>

class FloodVector3 {
	float x, y, z;

	FloodVector3() : x{ 0 }, y{ 0 }, z{ 0 }{ }
	FloodVector3(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
	// Todo:
	// Should add more math functions
};

class FloodVector2 {
public:
	float x, y;

	FloodVector2(): x{0}, y{0}{ }
	FloodVector2(float x, float y) { this->x = x; this->y = y; }
	// Todo:
	// Should add more math functions
};

class FloodColor {
public:
	uint8_t r, g, b, a;
	FloodColor() : r{ 0 }, g{ 0 }, b{ 0 }, a{ 255 } {};
	FloodColor(float r, float g, float b, float a = 1.f) { this->r = static_cast<int>(r * 255.f); this->g = static_cast<int>(g * 255.f); this->b = static_cast<int>(b * 255.f); this->a = static_cast<int>(a * 255.f);}
};