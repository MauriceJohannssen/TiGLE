#pragma once
#include <glm/vec3.hpp>

class Material
{
public:
	Material(const char* texturePath);
	Material(const Material &pMaterial);
	Material(const glm::vec3 pColor);
	Material();
	int GetTextureID() const;
	void Use() const;
	glm::vec3 GetColor() const;
	bool HasTexture() const;

private:
	unsigned int textureID;
	int width;
	int height;
	int numberChannels;
	glm::vec3 color;
};
