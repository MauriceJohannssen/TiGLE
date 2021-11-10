#pragma once
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include "Shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 UV;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
public:
	Mesh(std::vector<Vertex> pVertices, std::vector<unsigned int> pIndices, std::vector<Texture> pTextures);
	void Draw(Shader& pShader);

private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int VAO, VBO, EBO;
	void SetupGL();
};
