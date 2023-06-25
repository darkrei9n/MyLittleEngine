#include "Headers/vector.h"

Vector3 Vector3::vNorm(const Vector3& norm)
{
	if (norm.magnitude == 0)
	{
		return Vector3(0, 0, 0);
	}

	return Vector3(norm.x / norm.magnitude, norm.y / norm.magnitude, norm.z / norm.magnitude);
}

Vector3 Vector3::operator+(const Vector3& vector)
{
	return Vector3(x + vector.x, y + vector.y, z + vector.z);
}
Vector3 Vector3::operator-(const Vector3& vector)
{
	return Vector3(x - vector.x, y - vector.y, z - vector.z);
}

float Vector3::operator*(const Vector3& vector)
{
	return (x * vector.x + y * vector.y + z * vector.z);
}

bool Vector3::operator==(const Vector3& vector)
{
	if (abs((x - vector.x)) <= epsilon && abs((y - vector.y)) <= epsilon && abs((z - vector.z)) <= epsilon)
	{
		return true;
	}
	return false;
}

float Vector3::vAngle(const Vector3& vector)
{
	return acos(((*this) * vector) / (magnitude * vector.magnitude));
}

Vector3 Vector3::vCross(Vector3 vector1, Vector3 vector2)
{
	return Vector3(vector1.y*vector2.z - vector1.z*vector2.y, vector1.z * vector2.x - vector1.x * vector2.z, vector1.x * vector2.y - vector1.y * vector2.x);
}

void Vector3::setEpsilon(float eps)
{
	epsilon = eps;
}

float Vector3::vDot(const Vector3& a, const Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}