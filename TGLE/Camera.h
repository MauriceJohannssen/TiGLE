#pragma once
#include "GameObject.h"

class Camera : public GameObject
{
public:
	Camera();
	glm::vec3 GetForward() const;
	glm::vec3 GetUp() const;
	glm::vec3 GetPosition() const;
private:
};