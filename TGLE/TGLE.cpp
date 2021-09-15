#pragma once

#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <iostream>
#include "Input.h"
#include "Material.h"
#include "Shader.h"
#include "GameObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include <vector>

void DebugInformation();
void Render(const std::vector<GameObject> pGameObjects, const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const Shader shaderProgram);

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 3;
	settings.minorVersion = 3;

	sf::Window window(sf::VideoMode(1600, 900), "TGLE", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(true);
	window.setActive(true);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
	{
		std::cout << "Error: Could not initialize GLAD";
	}

	DebugInformation();

	glViewport(0, 0, 1600, 900);

	glEnable(GL_DEPTH_TEST);

	Shader shaderProgram("vertexShader.vert", "fragmentShader.frag");
	shaderProgram.Use();

	Material newMaterial("Pepe.jpg");
	newMaterial.Use();

	std::vector<GameObject> gameObjects;

	GameObject gameObject("TestGameObject", newMaterial);
	GameObject gameObject1("GO2", newMaterial);
	gameObject1.SetPosition(glm::vec3(2, 1, 4));

	gameObjects.push_back(gameObject);
	gameObjects.push_back(gameObject1);

	//Time
	sf::Clock clock;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	//Camera
	Camera mainCamera(ProjectionType::Perspective);
	mainCamera.SetPosition(glm::vec3(0, 0, 3));
	mainCamera.SetForward(glm::vec3(0, 0, -1));

	while (window.isOpen())
	{
		//Buffer Clearing
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update Time
		deltaTime = clock.getElapsedTime().asSeconds() - lastFrame;
		lastFrame = clock.getElapsedTime().asSeconds();

		//Input
		HandleInput(&window, &mainCamera, deltaTime);

		//Update MVP Matrix
		glm::mat4 view = glm::lookAt(mainCamera.GetPosition(), mainCamera.GetPosition() + mainCamera.GetForward(), mainCamera.GetUp());
		Render(gameObjects, view, mainCamera.GetProjectionMatrix(), shaderProgram);
		//Swap Buffers
		window.display();
	}

	window.close();

	return 0;
}

void Render(const std::vector<GameObject> pGameObjects, const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const Shader shaderProgram)
{
	for (GameObject element : pGameObjects)
	{
		glBindVertexArray(element.GetVAO());
		glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * element.GetObjectMatrix();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//Todo: Get vertex count here!
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}

//This is supposed to print all sorts of general debug information.
void DebugInformation()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes is: " << nrAttributes << std::endl;
}
