#include "Material.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "glad/glad.h"

Material::Material(const char* texturePath): textureID(0), textureIDSpecular(0), shininess(32)
{
	LoadTexture(texturePath, textureID, GL_RGB);
}

Material::Material()
{
	
}


Material::Material(const glm::vec3 pColor)
{
	
}

Material::Material(const Material &pMaterial)
{
	textureID = pMaterial.textureID;
	textureIDSpecular = pMaterial.textureIDSpecular;
	shininess = pMaterial.shininess;
}


unsigned int Material::GetTextureID() const
{
	return textureID;
}

void Material::Use() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureIDSpecular);
}


float Material::GetShininess() const
{
	return shininess;
}


bool Material::HasTexture() const
{
	return textureID != 0;
}

void Material::LoadTexture(const char* path, unsigned int& ID, GLenum format)
{
	int width = 0, height = 0, numberChannels = 0;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &numberChannels, 0);
	
	if (data)
	{
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		std::cout << "Failed to load texture." << std::endl;
	}

	stbi_image_free(data);
}

void Material::Add(const char* path)
{
	LoadTexture(path, textureIDSpecular, GL_RED);
}

unsigned int Material::GetSpecularID() const
{
	return textureIDSpecular;
}
