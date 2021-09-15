#pragma once
#include "GameObject.h"

enum ProjectionType
{
	Orthographic,
	Perspective
};

class Camera : public GameObject
{
public:
	Camera(ProjectionType pProjection);
	glm::mat4 GetProjectionMatrix() const;
private:
	glm::mat4 projectionMatrix;
};