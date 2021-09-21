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
	glm::vec3 GetLightColor() const;

private:
	LightType lightType;
	glm::vec3 color;
};
