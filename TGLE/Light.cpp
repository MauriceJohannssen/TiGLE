#include "Light.h"

Light::Light(std::string pName, LightType pLightType, glm::vec3 pColor) : GameObject(pName, new Material(pColor)), lightType(pLightType), ambient(pColor), diffuse(pColor), specular(pColor), intensity(1)
{
}

glm::vec3 Light::GetAmbient() const
{
	return ambient * 0.2f * intensity;
}

glm::vec3 Light::GetDiffuse() const
{
	return diffuse * intensity;
}

glm::vec3 Light::GetSpecular() const
{
	return specular * intensity;
}


