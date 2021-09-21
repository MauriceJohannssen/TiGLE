#include "Light.h"

Light::Light(std::string pName, LightType pLightType, glm::vec3 pColor) : GameObject(pName, new Material(pColor)), lightType(pLightType), color(pColor)
{
	
}

glm::vec3 Light::GetLightColor() const
{
	return color;
}

