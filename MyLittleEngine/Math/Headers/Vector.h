#pragma once
#include <cmath>

struct Vector3 {

	float x;
	float y;
	float z;
	float magnitude;
	Vector3() 
	{
		x = 0;
		y = 0;
		z = 0;
		magnitude = 0;
	}

	Vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;

		magnitude = sqrt(x*x + y*y + z*z);
	}

	static Vector3 vNorm(const Vector3& norm);
	Vector3 operator+(const Vector3& vector);
	Vector3 operator-(const Vector3& vector);
	//Vector3 operator=(const Vector3& vector);

	float operator*(const Vector3& vector);
	bool operator==(const Vector3& vector);

	inline float vAngle(const Vector3& vector1);
	static Vector3 vCross(Vector3 vector1, Vector3 vector2);
	static float vDot(const Vector3& a, const Vector3& b);

	//Maybe don't use this?
	void setEpsilon(float eps);


private:
	float epsilon = FLT_EPSILON;
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;

	Vector4()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;

	}

	Vector4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

struct Vector2 {
	float x;
	float y;

	Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
};