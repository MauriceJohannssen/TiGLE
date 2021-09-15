#include "GameObject.h"
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "glad/glad.h"

unsigned int GameObject::GameObjectCount = 0;

float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

GameObject::GameObject()
{
	//Keep track of deletions in destructor
	if(GameObjectCount == 0)
	{
		name = "gameObject_" + GameObjectCount;
	}
}

GameObject::GameObject(const std::string& name) : name(name)
{
	SetupGL();
	objectSpace = glm::mat4(1);
}

GameObject::GameObject(const std::string& pName, const Material &pMaterial) : name(pName), material(pMaterial)
{
	SetupGL();
	objectSpace = glm::mat4(1);
}

void GameObject::Translate(const glm::vec3 pDirection)
{
	//objectSpace = glm::translate(objectSpace, pDirection);
	//y tho?
	objectSpace[3] += glm::vec4(pDirection, 0);
}

void GameObject::SetPosition(const glm::vec3 pPosition)
{
	objectSpace = glm::translate(glm::mat4(1), pPosition);
}

void GameObject::SetForward(const glm::vec3 pForward)
{
	objectSpace[2] = glm::vec4(pForward, 0);
}


glm::mat4 GameObject::GetObjectMatrix() const
{
	return objectSpace;
}

glm::vec3 GameObject::GetForward()
{
	return objectSpace[2];
}

glm::vec3 GameObject::GetUp()
{
	return objectSpace[1];
}

glm::vec3 GameObject::GetPosition()
{
	return objectSpace[3];
}

void GameObject::SetupGL()
{
	//1. Create VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//2. Set Vertex Buffer Object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//3. Set Attribute Pointers
	//Vertices
	//Refers to layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	//1 is for color values
	//UVs
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//EBO HERE!
		//unsigned int EBO;
		//glGenBuffers(1, &EBO);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GameObject::Render(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const Shader shaderProgram) const
{
	glBindVertexArray(VAO);
	glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * objectSpace;
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

