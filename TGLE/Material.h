#pragma once

class Material
{
public:
	Material(const char* texturePath);
	Material();
	int GetTextureID() const;
	void Use() const;

private:
	unsigned int textureID;
	int width;
	int height;
	int numberChannels;
};
