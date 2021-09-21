#pragma once

#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
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
void Render(const std::vector<GameObject> pGameObjects, const std::vector<GameObject> pLights, 
	const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const Shader colorShader, const Shader textureShader);

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

	sf::Image icon;
	icon.loadFromFile("icon.png");
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
	{
		std::cout << "Error: Could not initialize GLAD";
	}

	DebugInformation();

	glViewport(0, 0, 1600, 900);

	glEnable(GL_DEPTH_TEST);

	Shader textureShader("vertexShader.vert", "textureShader.frag");
	Shader colorShader("vertexShader.vert", "colorShader.frag");

	textureShader.Use();
	textureShader.SetVec3("lightColor", glm::vec3(1, 1, 1));
	colorShader.Use();
	colorShader.SetVec3("lightColor", glm::vec3(1, 1, 1));

	Material textureMaterial("Pepe.jpg");
	Material colorMaterial(glm::vec3(0.0f, 1.0f, 0.0f));

	std::vector<GameObject> gameObjects;
	std::vector<GameObject> lightSources;

	GameObject gameObject("TextureMaterial", &textureMaterial);
	GameObject gameObject1("ColorMaterial", &colorMaterial);
	gameObject1.SetPosition(glm::vec3(2, 1, 4));
	
	//TODO: Create light class
	GameObject light("light", nullptr);
	light.SetPosition(glm::vec3(-1, 0, -1));
	light.Scale(glm::vec3(0.2f));

	gameObjects.push_back(gameObject);
	gameObjects.push_back(gameObject1);
	lightSources.push_back(light);

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
		Render(gameObjects, lightSources, view, mainCamera.GetProjectionMatrix(), colorShader, textureShader);
		//Swap Buffers
		window.display();
	}

	window.close();

	return 0;
}

void Render(const std::vector<GameObject> pGameObjects, const std::vector<GameObject> pLights, const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const Shader colorShader, const Shader textureShader)
{
	for (GameObject element : pGameObjects)
	{
		glBindVertexArray(element.GetVAO());
		glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * * element.GetObjectMatrix();
		glUniformMatrix4fv(glGetUniformLocation(colorShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
		if(element.GetMaterial().GetTextureID() == 0)
		{
			colorShader.Use();
			colorShader.SetVec3("objectColor", element.GetMaterial().GetColor());
		}
		else
		{
			textureShader.Use();
			element.GetMaterial().Use();
		}
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//Todo: Get vertex count here!
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	
	//TODO: Just temporary
	for (GameObject element : pLights)
	{
		colorShader.Use();
		glBindVertexArray(element.GetVAO());
		glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *element.GetObjectMatrix();
		glUniformMatrix4fv(glGetUniformLocation(textureShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
		colorShader.SetVec3("objectColor", element.GetMaterial().GetColor());
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
