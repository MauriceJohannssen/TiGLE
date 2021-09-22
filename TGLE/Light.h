#pragma once
#include "GameObject.h"
#include "Transform.h"

enum LightType
{
	Directional,
	Point,
	Spot
};

class Light : public GameObject
{
public:
	Light(std::string pName, LightType pLightType, glm::vec3 pColor);
	glm::vec3 GetAmbient() const;
	glm::vec3 GetDiffuse() const;
	glm::vec3 GetSpecular() const;

private:
	LightType lightType;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float intensity;
};
