#include "Light.h"

Light::Light(std::string pName, LightType pLightType, glm::vec3 pColor, const char* path, float pIntensity) : GameObject(path), lightType(pLightType), ambient(pColor), diffuse(pColor), specular(pColor), intensity(pIntensity)
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

LightType Light::GetLightType() const
{
	return lightType;
}

