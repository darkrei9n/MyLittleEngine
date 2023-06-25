#pragma once

#include "Vector.h"

struct Matrix44 {
	float m[4][4];

	Matrix44()
	{
		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
		m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
	}

	Matrix44(Vector4 row1, Vector4 row2, Vector4 row3, Vector4 row4)
	{
		m[0][0] = row1.x; m[0][1] = row1.y; m[0][2] = row1.z; m[0][3] = row1.w;
		m[1][0] = row2.x; m[1][1] = row2.y; m[1][2] = row2.z; m[1][3] = row2.w;
		m[2][0] = row3.x; m[2][1] = row3.y; m[2][2] = row3.z; m[2][3] = row3.w;
		m[3][0] = row4.x; m[3][1] = row4.y; m[3][2] = row4.z; m[3][3] = row4.w;
	}

	Matrix44(Vector3 row1, Vector3 row2, Vector3 row3, Vector3 row4) 
	{
		m[0][0] = row1.x; m[0][1] = row1.y; m[0][2] = row1.z; m[0][3] = 0;
		m[1][0] = row2.x; m[1][1] = row2.y; m[1][2] = row2.z; m[1][3] = 0;
		m[2][0] = row3.x; m[2][1] = row3.y; m[2][2] = row3.z; m[2][3] = 0;
		m[3][0] = row4.x; m[3][1] = row4.y; m[3][2] = row4.z; m[3][3] = 0;
	}

	Matrix44 operator*(const Matrix44& matrix);
	Matrix44 operator*(const float& scalar);
	Matrix44 operator+(const Matrix44& matrix);
	Matrix44 operator-(const Matrix44& matrix);
	Matrix44 operator/(const float& scalar);
	Matrix44 operator+=(const Matrix44& matrix);
	Matrix44 operator-=(const Matrix44& matrix);
	Matrix44 operator*=(const Matrix44& matrix);
	Matrix44 operator/=(const Matrix44& matrix);
	Matrix44 operator*=(const float& scalar);
	Matrix44 operator/=(const float& scalar);
	Matrix44 operator+(const float& scalar);
	Matrix44 operator-(const float& scalar);
	Matrix44 operator+=(const float& scalar);
	Matrix44 operator-=(const float& scalar);

};
