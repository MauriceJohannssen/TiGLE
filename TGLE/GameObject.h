#pragma once
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "Material.h"

class GameObject
{
public:
	GameObject();
	GameObject(const std::string &pName);
	GameObject(const std::string& pName, const Material &pMaterial);
	static unsigned int GameObjectCount;

	glm::vec3 GetPosition();
	glm::vec3 GetForward();
	glm::vec3 GetUp();
	glm::mat4 GetObjectMatrix() const;
	
	void Translate(const glm::vec3 pDirection);
	void SetPosition(const glm::vec3 pPosition);
	void SetForward(const glm::vec3 pForward);

private:
	std::string name;
	Material material;
	glm::mat4 objectSpace;
};