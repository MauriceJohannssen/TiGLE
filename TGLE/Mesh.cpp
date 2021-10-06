#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> pVertices, std::vector<unsigned int> pIndices, std::vector<Texture> pTextures): vertices(pVertices), indices(pIndices), textures(pTextures)
{
    SetupGL();
}

void Mesh::SetupGL()
{
	//1. Create VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//2. Set Vertex Buffer Object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	//3. Set Element Buffer Object
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	//3. Set Attribute Pointers
	//Vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));

	//Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	//UVs
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::Draw(Shader& pShader)
{
    unsigned int diffuseNumber = 1;
    unsigned int specularNumber = 1;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
        {
            number = std::to_string(diffuseNumber++);
        }
        else if (name == "texture_specular")
        {
            number = std::to_string(specularNumber++);
        }

        pShader.SetInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
	glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
