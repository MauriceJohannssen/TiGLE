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
	glm::vec3 GetForward();
	glm::vec3 GetUp();
	glm::vec3 GetPosition();
	glm::mat4 GetProjectionMatrix() const;
	void SetForward(const glm::vec3 pForwardVector);
private:
	glm::mat4 projectionMatrix;
};