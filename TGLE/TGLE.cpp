#pragma once

#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <stb_image.h>

#include "Input.h"
#include "Material.h"
#include "Shader.h"
#include "GameObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include <vector>
#include "Light.h"

void DebugInformation();
void Render(Camera& camera, std::vector<GameObject>& pGameObjects, const std::vector<Light>& pLights, const glm::mat4& viewMatrix, 
	const glm::mat4& projectionMatrix, const Shader& textureShader, const Shader& lightShader);

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

	window.setMouseCursorVisible(false);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
	{
		std::cout << "Error: Could not initialize GLAD";
	}

	DebugInformation();

	glViewport(0, 0, 1600, 900);

	glEnable(GL_DEPTH_TEST);

	Shader colorShader("vertexShader.vert", "colorShader.frag");
	Shader textureShader("vertexShader.vert", "textureShader.frag");
	Shader lightShader("vertexShader.vert", "lightShader.frag");

	std::vector<GameObject> gameObjects;
	std::vector<Light> lightSources;

	Material material("MetalColor.png");
	material.Add("MetalMetalness.png");

	GameObject gameObject("gameObject_1", &material);
	gameObject.SetPosition(glm::vec3(1.0f, -1.0f, 0.5f));

	Material material2("WoodFloorColor.png");
	material2.Add("WoodFloorRough.png");

	GameObject gameObject2("gameObject_2", &material2);
	gameObject2.SetPosition(glm::vec3(0.1f, -0.2f, -0.5f));
	gameObject2.Scale(glm::vec3(0.5f));

	gameObjects.push_back(gameObject);
	gameObjects.push_back(gameObject2);


	Light light("light_1", Point, glm::vec3(1.0f, 1.0f, 1.0f));
	light.SetPosition(glm::vec3(0, 0.5f, 0));
	light.Scale(glm::vec3(0.2f));

	lightSources.push_back(light);

	//Time
	sf::Clock clock;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	//Camera
	Camera mainCamera(Perspective);
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

		//Camera movement
		if (mainCamera.movementVector.length() > 0.01f)
		{
			mainCamera.Translate(mainCamera.movementVector * 0.2f * deltaTime);
			mainCamera.movementVector *= 0.9f;
		}

		//Update View Matrix
		glm::mat4 view = glm::lookAt(mainCamera.GetPosition(), mainCamera.GetPosition() + mainCamera.GetForward(), mainCamera.GetUp());
		
		//Render
		Render(mainCamera, gameObjects, lightSources, view, mainCamera.GetProjectionMatrix(), colorShader, lightShader);
		
		//Swap Buffers
		window.display();
	}

	window.close();

	return 0;
}

void Render(Camera& camera, std::vector<GameObject>& pGameObjects, const std::vector<Light>& pLights, 
	const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const Shader& textureShader, const Shader& lightShader)
{
	for(const Light& light : pLights)
	{
		glBindVertexArray(light.GetVAO());
		lightShader.Use();
		const glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *light.GetObjectMatrix();
		lightShader.SetMat4("transform", MVPMatrix);
		lightShader.SetVec3("lightColor",  glm::normalize(light.GetAmbient()));

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//Todo: Get vertex count here!
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	for (GameObject& gameObject : pGameObjects)
	{
		glBindVertexArray(gameObject.GetVAO());
		const glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *gameObject.GetObjectMatrix();

		Shader shader = textureShader;
		shader.Use();
		//Only set when texture exists.
		gameObject.GetMaterial().Use();
	
		shader.SetMat4("transform", MVPMatrix);
		shader.SetMat4("objectMatrix", *gameObject.GetObjectMatrix()); //Temporary

		//Object
		Material material = gameObject.GetMaterial();
		shader.SetInt("material.diffuse", 0);
		shader.SetInt("material.specular", 1);
		shader.SetFloat("material.shininess",  material.GetShininess());
		
		//Load in all lights here
		Light light = pLights.at(0);
		shader.SetVec3("light.lightPosition",light.GetPosition());
		shader.SetVec3("light.ambient", light.GetAmbient());
		shader.SetVec3("light.diffuse", light.GetDiffuse());
		shader.SetVec3("light.specular", light.GetSpecular());
		
		shader.SetVec3("cameraPosition", camera.GetPosition());

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
