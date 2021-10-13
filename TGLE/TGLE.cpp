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
	const glm::mat4& projectionMatrix, Shader& shader, Shader& lightShader, unsigned int fb, Shader& textureShader,
	unsigned int quadVAO, unsigned int textureColorBuffers[], unsigned int swappingFramebuffers[]
	, unsigned int swappingColorBuffers[], Shader& blurShader, Shader& bloomShader);

float quadVertices[] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

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

	//======================================================================================================================

	//Create a framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//Create attachment for the framebuffer, which is a texture in this case, since it must be read from.
	unsigned int textureColorbuffer[2];
	glGenTextures(2, textureColorbuffer);

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1600, 900, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		//No need to set mipmap option here.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//Bind to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureColorbuffer[i], 0);
	}

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	
	//Create Renderbuffer object for depth (and potentially stencil) values.
	unsigned int renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	//Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//======================================================================================================================

	//Create swapping framebuffers for Bloom
	unsigned int swappingBuffers[2];
	glGenFramebuffers(2, swappingBuffers);
	
	//Create two texture for the swapping frame buffers respectively.
	unsigned int swappingColorBuffers[2];
	glGenTextures(2, swappingColorBuffers);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, swappingBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, swappingColorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1600, 900, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		//No need to set mipmap option here.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, swappingColorBuffers[i], 0);
	}

	//======================================================================================================================

	//Create quad to render texture
	unsigned int quadVAO;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	
	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glBindVertexArray(0);

	//======================================================================================================================

	Shader colorShader("shader.vert", "shader.frag");
	Shader lightShader("shader.vert", "lightShader.frag");
	Shader textureShader("textureShader.vert", "textureShader.frag");
	textureShader.Use();
	textureShader.SetInt("screenTexture", 0);
	Shader blurShader("textureShader.vert", "gaussianBlur.frag");
	Shader bloomShader("textureShader.vert", "bloom.frag");
	bloomShader.Use();
	bloomShader.SetInt("hdrRender", 0);
	bloomShader.SetInt("bloomRender", 1);

	std::vector<GameObject> gameObjects;
	std::vector<Light> lightSources;

	//GameObjects
	GameObject gameObject1("Models/R99/R99.obj");
	gameObject1.SetPosition(glm::vec3(0));
	gameObjects.push_back(gameObject1);

	//Lights
	Light light("light_1", Point, glm::vec3(0.96f, 0.05f, 0.87f), "Models/Cube/Cube.obj", 3);
	light.SetPosition(glm::vec3(0.5f, 0.5f, 1));
	light.Scale(glm::vec3(0.1));
	lightSources.push_back(light);

	Light light2("light_2", Point, glm::vec3(0.0f, 0.31f, 0.95f), "Models/Cube/Cube.obj", 2);
	light2.SetPosition(glm::vec3(0.5f, 0.5, -1));
	light2.Scale(glm::vec3(0.1f));
	lightSources.push_back(light2);

	Light light3("light_3", Point, glm::vec3(0.24f, 0.95f, 0.13f), "Models/Cube/Cube.obj", 1);
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
		Render(mainCamera, gameObjects, lightSources, view, mainCamera.GetProjectionMatrix(), colorShader, lightShader, framebuffer, 
			textureShader, quadVAO, textureColorbuffer, swappingBuffers, swappingColorBuffers, blurShader, bloomShader);

		//Swap Buffers
		window.display();
	}

	window.close();

	return 0;
}

void Render(Camera& camera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights,
	const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Shader& shader, Shader& lightShader, unsigned int fb,
	Shader& textureShader, unsigned int quadVAO, unsigned int textureColorBuffers[], unsigned int swappingFramebuffers[]
	,unsigned int swappingColorBuffers[], Shader& blurShader, Shader& bloomShader)
{
	//Bind off-screen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (Light& light : pLights)
	{
		lightShader.Use();
		const glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * *light.GetObjectMatrix();
		lightShader.SetMat4("transform", MVPMatrix);
		lightShader.SetMat4("objectMatrix", *light.GetObjectMatrix());
		lightShader.SetVec3("lightColor", light.GetDiffuse()); 
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

	bool horizontal = true;
	bool firstIteration = true;
	int passes = 28;
	blurShader.Use();
	for (int i = 0; i < passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, swappingFramebuffers[horizontal]);
		blurShader.SetInt("horizontal", horizontal);
		glBindTexture(GL_TEXTURE_2D, firstIteration ? textureColorBuffers[1] : swappingColorBuffers[!horizontal]);
		
		//Render quad
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;
		if (firstIteration)
		{
			firstIteration = false;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	bloomShader.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, swappingColorBuffers[0]);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}

//This is supposed to print all sorts of general debug information.
void DebugInformation()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes is: " << nrAttributes << std::endl;
}
