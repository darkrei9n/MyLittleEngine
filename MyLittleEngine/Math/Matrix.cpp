#include "Headers/Matrix.h"

Matrix44 Matrix44::operator*(const Matrix44& matrix)
{
	Matrix44 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += m[i][k] * matrix.m[k][j];
			}
		}
	}
	return result;
}

Matrix44 Matrix44::operator*(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator+(const Matrix44& matrix)
{
	return Matrix44();
}

Matrix44 Matrix44::operator-(const Matrix44& matrix)
{
	return Matrix44();
}

Matrix44 Matrix44::operator/(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator+=(const Matrix44& matrix)
{
	return Matrix44();
}

Matrix44 Matrix44::operator-=(const Matrix44& matrix)
{
	return Matrix44();
}

Matrix44 Matrix44::operator*=(const Matrix44& matrix)
{
	return Matrix44();
}

Matrix44 Matrix44::operator/=(const Matrix44& matrix)
{
	return Matrix44();
}

Matrix44 Matrix44::operator*=(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator/=(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator+(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator-(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator+=(const float& scalar)
{
	return Matrix44();
}

Matrix44 Matrix44::operator-=(const float& scalar)
{
	return Matrix44();
}
