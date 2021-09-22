#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

class Material
{
public:
	
	Material(const char* texturePath);
	Material(const Material &pMaterial);
	Material(const glm::vec3 pColor);
	Material();
	unsigned int GetTextureID() const;
	unsigned int GetSpecularID() const;
	void Use() const;
	float GetShininess() const;
	bool HasTexture() const;
	unsigned int textureID, textureIDSpecular;
	void Add(const char* path);


private:
	void LoadTexture(const char* path, unsigned int& ID, GLenum format);
	float shininess;
};
