#pragma once
#include "Transform.h"
#include "GameObject.h"

enum LightType
{
	Directional,
	Point,
	Spot
};

class Light : public GameObject
{
public:
	Light(std::string pName, LightType pLightType, glm::vec3 pColor, const char* path, float pIntensity = 1.0f);
	glm::vec3 GetAmbient() const;
	glm::vec3 GetDiffuse() const;
	glm::vec3 GetSpecular() const;
	LightType GetLightType() const;

private:
	LightType lightType;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float intensity;
};
