#include "Camera.h"

Camera::Camera() : GameObject("Camera")
{
	
}

glm::vec3 Camera::GetForward() const
{
	return GetObjectMatrix()[2];
}

glm::vec3 Camera::GetUp() const
{
	return GetObjectMatrix()[1];
}

glm::vec3 Camera::GetPosition() const
{
	return GetObjectMatrix()[3];
}

