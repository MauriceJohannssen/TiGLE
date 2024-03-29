#include "Camera.h"

Camera::Camera(ProjectionType pProjection) : movementVector(glm::vec3(0)), nearFarPlane(0.1f, 250.0f) {
	if (pProjection == ProjectionType::Orthographic) {
		projectionMatrix = glm::ortho(0.0f, 1600.0f, 0.0f, 900.0f, 0.1f, 100.0f);
	}
	else {
		projectionMatrix = glm::perspective(glm::radians(50.0f), 1600.0f / 800.0f, 0.1f, 250.0f);
	}
}

glm::mat4 Camera::GetProjectionMatrix() const {
	return projectionMatrix;
}

glm::vec3 Camera::GetMovementVector() const {
	return movementVector;
}

void Camera::SetMovementVector(const glm::vec3 vector) {
	movementVector = vector;
}

std::tuple<float, float> Camera::GetNearFarPlanes() {
	return nearFarPlane;
}
