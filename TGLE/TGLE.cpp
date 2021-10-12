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
void Render(Camera& camera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights, const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix, Shader& shader, Shader& lightShader);

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 3;
	settings.minorVersion = 3;

	sf::Window window(sf::VideoMode(1600, 900), "TiGLE", sf::Style::Default, settings);
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
	Shader lightShader("vertexShader.vert", "lightShader.frag");

	std::vector<GameObject> gameObjects;
	std::vector<Light> lightSources;

	//GameObjects
	GameObject gameObject1("Models/R99/R99.obj");
	gameObject1.SetPosition(glm::vec3(0));
	gameObjects.push_back(gameObject1);

	//Lights
	Light light("light_1", Point, glm::vec3(242, 19, 215) / 255.0f, "Models/Cube/Cube.obj");
	light.SetPosition(glm::vec3(0.5f, 0.5f, 1));
	light.Scale(glm::vec3(0.1));
	lightSources.push_back(light);

	Light light2("light_2", Point, glm::vec3(0, 78, 235) / 255.0f, "Models/Cube/Cube.obj");
	light2.SetPosition(glm::vec3(0.5f, 0.5, -1));
	light2.Scale(glm::vec3(0.1f));
	lightSources.push_back(light2);

	Light light3("light_3", Point, glm::vec3(56, 240, 70) / 255.0f, "Models/Cube/Cube.obj");
	light3.SetPosition(glm::vec3(1, 0, 0));
	light3.Scale(glm::vec3(0.1f));
	lightSources.push_back(light3);

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
			mainCamera.Translate(mainCamera.movementVector * 0.8f * deltaTime);
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

void Render(Camera& camera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights,
	const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Shader& shader, Shader& lightShader)
{
	for (Light& light : pLights)
	{
		lightShader.Use();
		const glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *light.GetObjectMatrix();
		lightShader.SetMat4("transform", MVPMatrix);
		lightShader.SetMat4("objectMatrix", *light.GetObjectMatrix());
		lightShader.SetVec3("lightColor", glm::normalize(light.GetAmbient()));
		light.Draw(lightShader);
	}


	for (GameObject& gameObject : pGameObjects)
	{
		shader.Use();
		const glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *gameObject.GetObjectMatrix();
		shader.SetMat4("transform", MVPMatrix);
		shader.SetMat4("objectMatrix", *gameObject.GetObjectMatrix());
		shader.SetVec3("cameraPosition", camera.GetPosition());

		//This is inefficient and just for testing purposes!
		//Use uniform buffer objects or classes.

		Light light = pLights.at(0);
		shader.SetVec3("pointLights[0].position", light.GetPosition());
		shader.SetVec3("pointLights[0].ambient", light.GetAmbient());
		shader.SetVec3("pointLights[0].diffuse", light.GetDiffuse());
		shader.SetVec3("pointLights[0].specular", light.GetSpecular());
		shader.SetFloat("pointLights[0].constant", 1.0f);
		shader.SetFloat("pointLights[0].linear", 0.07f);
		shader.SetFloat("pointLights[0].quadratic", 0.3f);

		light = pLights.at(1);
		shader.SetVec3("pointLights[1].position", light.GetPosition());
		shader.SetVec3("pointLights[1].ambient", light.GetAmbient());
		shader.SetVec3("pointLights[1].diffuse", light.GetDiffuse());
		shader.SetVec3("pointLights[1].specular", light.GetSpecular());
		shader.SetFloat("pointLights[1].constant", 1.0f);
		shader.SetFloat("pointLights[1].linear", 0.07f);
		shader.SetFloat("pointLights[1].quadratic", 0.3f);

		light = pLights.at(2);
		shader.SetVec3("pointLights[2].position", light.GetPosition());
		shader.SetVec3("pointLights[2].ambient", light.GetAmbient());
		shader.SetVec3("pointLights[2].diffuse", light.GetDiffuse());
		shader.SetVec3("pointLights[2].specular", light.GetSpecular());
		shader.SetFloat("pointLights[2].constant", 1.0f);
		shader.SetFloat("pointLights[2].linear", 0.07f);
		shader.SetFloat("pointLights[2].quadratic", 0.3f);

		gameObject.Draw(shader);
	}
}

//This is supposed to print all sorts of general debug information.
void DebugInformation()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes is: " << nrAttributes << std::endl;
}
