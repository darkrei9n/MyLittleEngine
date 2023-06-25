#pragma once

#include "../../Utility/headers/MathStructures.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi


struct ViewProjection
{
	Matrix44 viewMatrix;
	Matrix44 projMatrix;
};

class Camera
{
	Vector3 position;
	Vector3 zaxis;
	Vector3 yaxis;
	Vector3 xaxis;

public:
	ViewProjection viewProj;
    Camera(Vector3 pos, Vector3 target, Vector3 up, int height, int width, float clipClose, float clipFar)
    {
        this->position = pos;

        zaxis = Vector3::vNorm(pos - target);
        xaxis = Vector3::vNorm(Vector3::vCross(up, zaxis));
        yaxis = Vector3::vCross(zaxis, xaxis);

        float ratio = (float)width / (float)height;
        float f = 1.0f / std::tan((45) * 3.14159 / 180.0f);

        float x = f / ratio;
        float y = -f;
        float z = clipFar / (clipClose - clipFar);
        float w = (clipClose * clipFar) / (clipClose - clipFar);

        viewProj.projMatrix = Matrix44(
            Vector4(x, 0, 0, 0),
            Vector4(0, y, 0, 0),
            Vector4(0, 0, z, -1.0f),
            Vector4(0, 0, w, 0)
        );

        viewProj.viewMatrix = Matrix44(
            Vector4(xaxis.x, yaxis.x, -zaxis.x, 0),
            Vector4(xaxis.y, yaxis.y, -zaxis.y, 0),
            Vector4(xaxis.z, yaxis.z, -zaxis.z, 0),
            Vector4(-Vector3::vDot(xaxis, position), -Vector3::vDot(yaxis, position), -Vector3::vDot(zaxis, position), 1)
        );
    }

};