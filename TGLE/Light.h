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

private:
	LightType lightType;
};
