#include "Camera.h"

Camera::Camera(ProjectionType pProjection) : movementVector(glm::vec3(0))
{
	if(pProjection == ProjectionType::Orthographic)
	{
		projectionMatrix = glm::ortho(0.0f, 1600.0f, 0.0f, 900.0f, 0.1f, 100.0f);
	}
	else
	{
		projectionMatrix = glm::perspective(glm::radians(50.0f), 1600.0f / 800.0f, 0.1f, 100.0f);
	}
}


glm::mat4 Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

