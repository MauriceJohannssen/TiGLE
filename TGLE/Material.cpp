#include "Material.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "glad/glad.h"

Material::Material(const char* texturePath)
{
	textureID = 0;
	width = 0;
	height = 0;
	numberChannels = 0;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(texturePath, &width, &height, &numberChannels, 0);

	if(data)
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		std::cout << "Failed to load texture." << std::endl;
	}
	
	stbi_image_free(data);
}

Material::Material(const Material &pMaterial)
{
	textureID = pMaterial.textureID;
	width = pMaterial.width;
	height = pMaterial.height;
	numberChannels = pMaterial.numberChannels;
}

Material::Material()
{
	
}



int Material::GetTextureID() const
{
	return textureID;
}

void Material::Use() const
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}
