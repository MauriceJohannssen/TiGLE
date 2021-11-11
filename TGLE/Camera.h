#pragma once
#include "Transform.h"

enum class ProjectionType {
	Orthographic,
	Perspective
};

class Camera : public Transform {
public:
	Camera(ProjectionType pProjection);
	glm::mat4 GetProjectionMatrix() const;
	glm::vec3 GetMovementVector() const;
	void SetMovementVector(const glm::vec3 pVector);

private:
	glm::vec3 movementVector;
	glm::mat4 projectionMatrix;

};
