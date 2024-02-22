#pragma once
#include <cstdint>

class FLOODVector3 {
	float x, y, z;

	FLOODVector3() : x{ 0 }, y{ 0 }, z{ 0 }{ }
	FLOODVector3(float x = 0.f, float y = 0.f, float z = 0.f) { this->x = x; this->y = y; this->z = z; }
	FLOODVector3(int x = 0, int y = 0, int z = 0) { this->x = x; this->y = y; this->z = z; }
	// Todo:
	// Should add more math functions
};

class FLOODVector2 {
public:
	float x, y;

	FLOODVector2(): x{0}, y{0}{ }
	FLOODVector2(float x = 0.f, float y = 0.f) { this->x = x; this->y = y; }
	// Todo:
	// Should add more math functions
};

class FLOODColor {
public:
	uint8_t r, g, b, a;
	FLOODColor() : r{ 0 }, g{ 0 }, b{ 0 }, a{ 1 } {};
	FLOODColor(float r, float g, float b, float a = 1.f) { this->r = static_cast<int>(r * 255.f); this->g = static_cast<int>(g * 255.f); this->b = static_cast<int>(b * 255.f); this->a = static_cast<int>(a * 255.f);}
};