#include "Camera.h"

Camera::Camera(ProjectionType pProjection) : GameObject("Camera")
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

glm::vec3 Camera::GetForward()
{
	return GetObjectMatrix()[2];
}

void Camera::SetForward(const glm::vec3 pForwardVector)
{
	objectSpace[2] = glm::vec4(pForwardVector, 0);
}

glm::vec3 Camera::GetUp()
{
	return GetObjectMatrix()[1];
}

glm::vec3 Camera::GetPosition()
{
	glm::vec4 test = GetObjectMatrix()[3];
	glm::vec3 newTest;
	newTest.x = test.x;
	newTest.y = test.y;
	newTest.z = test.z;
	return newTest;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

