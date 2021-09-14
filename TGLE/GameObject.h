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
	void Translate(const glm::vec3 pDirection);
	void SetPosition(const glm::vec3 pPosition);
	glm::mat4 GetObjectMatrix() const;
private:
	glm::mat4 objectSpace;
	std::string name;
	Material material;
};