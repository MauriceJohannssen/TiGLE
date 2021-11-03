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
	void SetAmbient(const glm::vec3 pColor);
	glm::vec3 GetDiffuse() const;
	void SetDiffuse(const glm::vec3 pColor);
	glm::vec3 GetSpecular() const;
	void SetSpecular(const glm::vec3 pColor);
	float GetIntensity() const;
	void SetIntensity(const float pIntensity);
	LightType GetLightType() const;

private:
	LightType lightType;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float intensity;
};
