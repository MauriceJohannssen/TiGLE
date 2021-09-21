#pragma once
#include "Transform.h"

enum ProjectionType
{
	Orthographic,
	Perspective
};

class Camera : public Transform
{
public:
	Camera(ProjectionType pProjection);
	glm::mat4 GetProjectionMatrix() const;
	glm::vec3 movementVector;
private:
	glm::mat4 projectionMatrix;

};