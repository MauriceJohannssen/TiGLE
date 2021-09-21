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

#include "Light.h"

void DebugInformation();
void Render(const std::vector<GameObject>& pGameObjects, const std::vector<Light>& pLights, 
	const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const Shader& colorShader, const Shader& textureShader, const Shader& lightShader, Camera& camera);

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

	Shader textureShader("vertexShader.vert", "textureShader.frag");
	Shader colorShader("vertexShader.vert", "colorShader.frag");
	Shader lightShader("vertexShader.vert", "lightShader.frag");

	textureShader.Use();
	textureShader.SetVec3("lightColor", glm::vec3(1, 1, 1));
	colorShader.Use();
	colorShader.SetVec3("lightColor", glm::vec3(1, 1, 1));
	lightShader.Use();
	colorShader.SetVec3("lightColor", glm::vec3(1, 1, 1));

	Material textureMaterial(glm::vec3(0.3f, 1.0f, 0.3f));
	Material colorMaterial(glm::vec3(0.3f, 0.2f, 0.8f));

	std::vector<GameObject> gameObjects;
	std::vector<Light> lightSources;

	GameObject gameObject("TextureMaterial", &textureMaterial);
	gameObject.SetPosition(glm::vec3(1.4f, -1.f, 0.5f));
	GameObject gameObject1("ColorMaterial", &colorMaterial);
	gameObject1.SetPosition(glm::vec3(-2, 1, -1));
	
	//TODO: Create light class
	Light light("light1", Point,glm::vec3(1.0f, 1.0f ,1.0f));
	light.SetPosition(glm::vec3(0, 0, 0));
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
		Render(gameObjects, lightSources, view, mainCamera.GetProjectionMatrix(), colorShader, textureShader, lightShader , mainCamera);
		//Swap Buffers
		window.display();
	}

	window.close();

	return 0;
}

void Render(const std::vector<GameObject>& pGameObjects, const std::vector<Light>& pLights, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const Shader& colorShader, const Shader& textureShader, const Shader& lightShader, Camera& camera)
{
	for (GameObject element : pGameObjects)
	{
		glBindVertexArray(element.GetVAO());
		glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *element.GetObjectMatrix();
		if(element.GetMaterial().GetTextureID() == 0)
		{
			colorShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(colorShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
			glUniformMatrix4fv(glGetUniformLocation(colorShader.ID, "objectMatrix"), 1, GL_FALSE, glm::value_ptr( *element.GetObjectMatrix()));
			colorShader.SetVec3("objectColor", element.GetMaterial().GetColor());
			colorShader.SetVec3("lightPosition", pLights.at(0).GetPosition());
			colorShader.SetVec3("cameraPosition", camera.GetPosition());
		}
		else
		{
			textureShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(textureShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
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
		lightShader.Use();
		glBindVertexArray(element.GetVAO());
		glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *element.GetObjectMatrix();
		glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(MVPMatrix));
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
