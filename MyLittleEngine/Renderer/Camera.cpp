#include "Headers/Camera.h"

void Camera::translate(Vector3 translation)
{
    // Update the position of the camera
    position = position + translation;

    // Only update the translation part of the view matrix
    viewProj.viewMatrix.m[3][0] = -Vector3::vDot(xaxis, position);
    viewProj.viewMatrix.m[3][1] = -Vector3::vDot(yaxis, position);
    viewProj.viewMatrix.m[3][2] = -Vector3::vDot(zaxis, position);
}

void Camera::rotate()
{

}