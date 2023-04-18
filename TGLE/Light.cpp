#include "Light.h"

Light::Light(std::string pName, LightType pLightType, glm::vec3 pColor, const char* path, float pIntensity) : 
	GameObject(path), lightType(pLightType), ambient(pColor), diffuse(pColor), specular(pColor), intensity(pIntensity) {

}

glm::vec3 Light::GetAmbient() const {
	return ambient;
}

void Light::SetAmbient(glm::vec3 pColor) {
	ambient = pColor;
}

glm::vec3 Light::GetDiffuse() const {
	return diffuse;
}

void Light::SetDiffuse(const glm::vec3 pColor) {
	diffuse = pColor;
}

glm::vec3 Light::GetSpecular() const {
	return specular;
}

void Light::SetSpecular(const glm::vec3 pColor) {
	specular = pColor;
}

float Light::GetIntensity() const {
	return intensity;
}

void Light::SetIntensity(const float pIntensity) {
	if (pIntensity < 0) return;
	intensity = pIntensity;
}

LightType Light::GetLightType() const {
	return lightType;
}
