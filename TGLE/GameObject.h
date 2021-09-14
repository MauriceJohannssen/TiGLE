#pragma once
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject
{
public:
	GameObject();
	void Translate(glm::vec3 pDirection);
private:
	glm::mat4 objectSpace;
};