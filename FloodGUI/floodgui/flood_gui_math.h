#pragma once
#include <cstdint>

class FloodVector4 {
public:
	float x, y, z, w;
	FloodVector4() : x{ 0 }, y{ 0 }, z{ 0 }, w{0}{ }
	FloodVector4(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	FloodVector4 operator+(const FloodVector4& other) noexcept
	{
		return FloodVector4(this->x + other.x,
			this->y + other.y,
			this->z + other.z,
			this->w + other.w);
	}

	FloodVector4 operator-(const FloodVector4& other) noexcept
	{
		return FloodVector4(this->x - other.x,
			this->y - other.y,
			this->z - other.z,
			this->w - other.w);
	}

	FloodVector4 operator*(const FloodVector4& other) noexcept
	{
		return FloodVector4(this->x * other.x,
			this->y * other.y,
			this->z * other.z,
			this->w * other.w);
	}

	FloodVector4 operator/(const FloodVector4& other) noexcept
	{
		return FloodVector4(this->x / other.x,
			this->y / other.y,
			this->z / other.z,
			this->w / other.w);
	}

	FloodVector4 operator+(const float& other) noexcept
	{
		return FloodVector4(this->x + other,
			this->y + other,
			this->z + other,
			this->w + other);
	}

	FloodVector4 operator-(const float& other) noexcept
	{
		return FloodVector4(this->x - other,
			this->y - other,
			this->z - other,
			this->w - other);
	}

	FloodVector4 operator*(const float& other) noexcept
	{
		return FloodVector4(this->x * other,
			this->y * other,
			this->z * other,
			this->w * other);
	}

	FloodVector4 operator/(const float& other) noexcept
	{
		return FloodVector4(this->x / other,
			this->y / other,
			this->z / other,
			this->w / other);
	}
};

class FloodVector3 {
public:
	float x, y, z;
	FloodVector3() : x{ 0 }, y{ 0 }, z{ 0 }{ }
	FloodVector3(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
	FloodVector3 operator+(const FloodVector3& other) noexcept
	{
		return FloodVector3(this->x + other.x,
			this->y + other.y,
			this->z + other.z);
	}

	FloodVector3 operator-(const FloodVector3& other) noexcept
	{
		return FloodVector3(this->x - other.x,
			this->y - other.y,
			this->z - other.z);
	}

	FloodVector3 operator*(const FloodVector3& other) noexcept
	{
		return FloodVector3(this->x * other.x,
			this->y * other.y,
			this->z * other.z);
	}

	FloodVector3 operator/(const FloodVector3& other) noexcept
	{
		return FloodVector3(this->x / other.x,
			this->y / other.y,
			this->z / other.z);
	}

	FloodVector3 operator+(const float& other) noexcept
	{
		return FloodVector3(this->x + other,
			this->y + other,
			this->z + other);
	}

	FloodVector3 operator-(const float& other) noexcept
	{
		return FloodVector3(this->x - other,
			this->y - other,
			this->z - other);
	}

	FloodVector3 operator*(const float& other) noexcept
	{
		return FloodVector3(this->x * other,
			this->y * other,
			this->z * other);
	}

	FloodVector3 operator/(const float& other) noexcept
	{
		return FloodVector3(this->x / other,
			this->y / other,
			this->z / other);
	}
};

class FloodVector2 {
public:
	float x, y;
	FloodVector2(): x{0}, y{0}{ }
	FloodVector2(float x, float y) { this->x = x; this->y = y; }
	FloodVector2 operator+(const FloodVector2& other) noexcept
	{
		return FloodVector2(this->x + other.x,
			this->y + other.y);
	}

	FloodVector2 operator-(const FloodVector2& other) noexcept
	{
		return FloodVector2(this->x - other.x,
			this->y - other.y);
	}

	FloodVector2 operator*(const FloodVector2& other) noexcept
	{
		return FloodVector2(this->x * other.x,
			this->y * other.y);
	}

	FloodVector2 operator/(const FloodVector2& other) noexcept
	{
		return FloodVector2(this->x / other.x,
			this->y / other.y);
	}

	FloodVector2 operator+(const float& other) noexcept
	{
		return FloodVector2(this->x + other,
			this->y + other);
	}

	FloodVector2 operator-(const float& other) noexcept
	{
		return FloodVector2(this->x - other,
			this->y - other);
	}

	FloodVector2 operator*(const float& other) noexcept
	{
		return FloodVector2(this->x * other,
			this->y * other);
	}

	FloodVector2 operator/(const float& other) noexcept
	{
		return FloodVector2(this->x / other,
			this->y / other);
	}
};

class FloodColor {
private:
	uint8_t rdata[4] = {0, 0, 0, 255};
public:
	FloodColor() {  }
	FloodColor(float r, float g, float b, float a = 1.f) { rdata[0] = static_cast<int>(r * 255.f); rdata[1] = static_cast<int>(g * 255.f); rdata[2] = static_cast<int>(b * 255.f); rdata[3] = static_cast<int>(a * 255.f); }
	FloodColor(int r, int g, int b, int a) { rdata[0] = r; rdata[1] = g; rdata[2] = b; rdata[3] = a; }
	~FloodColor() {  }
	constexpr uint8_t& r() { return rdata[0]; }
	constexpr uint8_t& g() { return rdata[1]; }
	constexpr uint8_t& b() { return rdata[2]; }
	constexpr uint8_t& a() { return rdata[3]; }
	constexpr unsigned int ToU32()
	{
		return ((BYTE(this->a()) << 24) + (BYTE(this->r()) << 16) + (BYTE(this->g()) << 8) + BYTE(this->b()));
	}
	constexpr uint8_t* data() { return rdata; }
};

constexpr bool FindPoint(const FloodVector2& min, const FloodVector2& max, const FloodVector2& point)
{
	return (point.x > min.x && point.x < max.x&& point.y > min.y && point.y < max.y);
}