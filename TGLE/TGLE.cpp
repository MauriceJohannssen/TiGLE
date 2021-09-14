#pragma once

#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <iostream>
#include "Input.h"
#include "Material.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void DebugInformation();


int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 3;
	settings.minorVersion = 3;

	sf::Window window(sf::VideoMode(1600, 900), "TGLE", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(false);
	window.setActive(true);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
	{
		std::cout << "Could not initialize GLAD";
	}

	DebugInformation();

	glViewport(0, 0, 1600, 900);

	float vertices[] = {
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 1.0f, 0.2f, 1.0f, 0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	Shader shaderProgram("vertexShader.vert", "fragmentShader.frag");
	shaderProgram.Use();

	Material newMaterial("Pepe.jpg");
	newMaterial.Use();

	glm::vec4 vec(1, 0, 0, 1);
	glm::mat4 translation = glm::mat4(1.0f);
	translation = glm::translate(translation, glm::vec3(1.0f, 1.0f, 0.0f));
	vec = translation * vec;

	unsigned int transformMatrixID = glGetUniformLocation(shaderProgram.ID, "transform");
	sf::Clock clock;

	while (window.isOpen())
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		HandleInput(&window);

		//Copy constructor called
		glm::mat4 rotationScaling = glm::mat4(1.0f);
		rotationScaling = glm::rotate(rotationScaling, clock.getElapsedTime().asSeconds(), glm::vec3(0, 0, 1.0f));
		rotationScaling = glm::scale(rotationScaling, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(transformMatrixID, 1, GL_FALSE, glm::value_ptr(rotationScaling));

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		window.display();
	}

	window.close();

	return 0;
}

void DebugInformation()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes is: " << nrAttributes << std::endl;
}
