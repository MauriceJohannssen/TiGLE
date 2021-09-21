#pragma once
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "Material.h"
#include "Shader.h"
#include "Transform.h"

class GameObject : public Transform
{
public:
	GameObject();
	GameObject(std::string pName, Material* pMaterial= nullptr);

	unsigned int GetVAO() const;
	Material GetMaterial() const;

private:
	Material material;
	unsigned int VAO;
	unsigned int VBO;

	void SetupGL();
};
