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
#include <map>
#include "Light.h"

int windowWidth = 1600;
int windowHeight = 900;

void DebugInformation();
void CreateHDRBuffers(unsigned int& pFramebuffer, unsigned int pColorbuffers[], unsigned int& pRenderbuffer, unsigned int& pVertexPosition);
void CreateBloomBuffers(unsigned int pFramebuffers[], unsigned int pColorbuffers[]);
void CreateRenderQuad(unsigned int& pVAO);

void Render(Camera& pCamera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights, const glm::mat4& pViewMatrix,const glm::mat4& pProjectionMatrix, 
	std::map<std::string, Shader> pShaders, unsigned int pQuadVAO, unsigned int pHdrFramebuffer, unsigned int pHdrColorbuffers[], unsigned int pBloomFramebuffer[], unsigned int pBloomColorbuffers[], 
	unsigned int& pVertexPositions);


//These vertices are being used for the rendering quad used for HDR & Bloom.
float quadVertices[] = {
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

	sf::Window window(sf::VideoMode(windowWidth, windowHeight), "TiGLE", sf::Style::Default, settings);
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

	glViewport(0, 0, windowWidth, windowHeight);

	//Setup buffers
	unsigned int hdrFramebuffer;
	unsigned int hdrColorbuffers[2];
	unsigned int vertexPosition;
	unsigned int hdrRenderbuffer;

	CreateHDRBuffers(hdrFramebuffer, hdrColorbuffers, hdrRenderbuffer, vertexPosition);

	unsigned int bloomFramebuffers[2];
	unsigned int bloomColorbuffers[2];
	CreateBloomBuffers(bloomFramebuffers, bloomColorbuffers);

	unsigned int quadVAO;
	CreateRenderQuad(quadVAO);

	//Setup shaders
	std::map<std::string, Shader> shaders;
	shaders["colorShader"] = Shader("shader.vert", "shader.frag");
	shaders["lightShader"] = Shader("shader.vert", "lightShader.frag");

	Shader textureShader("textureShader.vert", "textureShader.frag");
	textureShader.Use();
	textureShader.SetInt("screenTexture", 0);
	shaders["textureShader"] = textureShader;

	shaders["blurShader"] = Shader("textureShader.vert", "gaussianBlurShader.frag");

	Shader bloomShader("textureShader.vert", "bloomShader.frag");
	bloomShader.Use();
	bloomShader.SetInt("hdrRender", 0);
	bloomShader.SetInt("bloomRender", 1);

	shaders["bloomShader"] = bloomShader;

	Shader DoFShader("textureShader.vert", "DoFShader.frag");
	DoFShader.Use();
	DoFShader.SetInt("sharpTexture", 0);
	DoFShader.SetInt("blurredTexture", 1);
	DoFShader.SetInt("vertexPositions", 2);
	shaders["dofShader"] = DoFShader;

	std::vector<GameObject> gameObjects;
	std::vector<Light> lightSources;

	//GameObjects
	GameObject gameObject1("Models/R99/R99.obj");
	gameObject1.SetPosition(glm::vec3(-0.5f, -0.25f, 0));
	gameObject1.Scale(glm::vec3(2));
	gameObjects.push_back(gameObject1);

	//Lights
	Light light("light_1", Point, glm::vec3(0.96f, 0.05f, 0.87f), "Models/Cube/Cube.obj", 3);
	light.SetPosition(glm::vec3(0.5f, 0.5f, 1));
	light.Scale(glm::vec3(0.05f));
	lightSources.push_back(light);

	Light light2("light_2", Point, glm::vec3(0.0f, 0.31f, 0.95f), "Models/Cube/Cube.obj", 2);
	light2.SetPosition(glm::vec3(0.2f, 0.5, -1));
	light2.Scale(glm::vec3(0.05f));
	lightSources.push_back(light2);

	Light light3("light_3", Point, glm::vec3(0.24f, 0.95f, 0.13f), "Models/Cube/Cube.obj", 1);
	light3.SetPosition(glm::vec3(1, 0, 0));
	light3.Scale(glm::vec3(0.05f));
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
		//Update Time
		deltaTime = clock.getElapsedTime().asSeconds() - lastFrame;
		lastFrame = clock.getElapsedTime().asSeconds();

		//Input
		HandleInput(&window, &mainCamera, deltaTime);

		//Camera movement
		if (mainCamera.GetMovementVector().length() > 0.01f)
		{
			mainCamera.Translate(mainCamera.GetMovementVector() * 0.8f * deltaTime);
			mainCamera.SetMovementVector(mainCamera.GetMovementVector() * 0.9f);
		}

		//Update View Matrix
		glm::mat4 view = glm::lookAt(mainCamera.GetPosition(), mainCamera.GetPosition() + mainCamera.GetForward(), mainCamera.GetUp());

		//Render
		Render(mainCamera, gameObjects, lightSources, view, mainCamera.GetProjectionMatrix(), shaders, quadVAO, hdrFramebuffer, hdrColorbuffers, bloomFramebuffers, bloomColorbuffers, vertexPosition);

		//Swap Buffers
		window.display();
	}

	window.close();

	return 0;
}

void Render(Camera& pCamera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix,
	std::map<std::string, Shader> pShaders, unsigned int pQuadVAO, unsigned int pHdrFramebuffer, unsigned int pHdrColorbuffers[], unsigned int pBloomFramebuffer[], unsigned int pBloomColorbuffers[], 
	unsigned int& pVertexPositions)
{
	//Bind off-screen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pHdrFramebuffer);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (Light& light : pLights)
	{
		pShaders["lightShader"].Use();
		const glm::mat4 MVPMatrix = pProjectionMatrix * pViewMatrix * *light.GetObjectMatrix();
		pShaders["lightShader"].SetMat4("transform", MVPMatrix);
		pShaders["lightShader"].SetMat4("objectMatrix", *light.GetObjectMatrix());
		pShaders["lightShader"].SetVec3("lightColor", light.GetDiffuse());
		light.Draw(pShaders["lightShader"]);
	}


	for (GameObject& gameObject : pGameObjects)
	{
		pShaders["colorShader"].Use();
		const glm::mat4 MVPMatrix = pProjectionMatrix * pViewMatrix * *gameObject.GetObjectMatrix();
		pShaders["colorShader"].SetMat4("transform", MVPMatrix);
		pShaders["colorShader"].SetMat4("objectMatrix", *gameObject.GetObjectMatrix());
		pShaders["colorShader"].SetVec3("cameraPosition", pCamera.GetPosition());

		//This is inefficient and just for testing purposes!
		//Use uniform buffer objects or classes.

		Light light = pLights.at(0);
		pShaders["colorShader"].SetVec3("pointLights[0].position", light.GetPosition());
		pShaders["colorShader"].SetVec3("pointLights[0].ambient", light.GetAmbient());
		pShaders["colorShader"].SetVec3("pointLights[0].diffuse", light.GetDiffuse());
		pShaders["colorShader"].SetVec3("pointLights[0].specular", light.GetSpecular());
		pShaders["colorShader"].SetFloat("pointLights[0].constant", 1.0f);
		pShaders["colorShader"].SetFloat("pointLights[0].linear", 0.07f);
		pShaders["colorShader"].SetFloat("pointLights[0].quadratic", 0.3f);

		light = pLights.at(1);
		pShaders["colorShader"].SetVec3("pointLights[1].position", light.GetPosition());
		pShaders["colorShader"].SetVec3("pointLights[1].ambient", light.GetAmbient());
		pShaders["colorShader"].SetVec3("pointLights[1].diffuse", light.GetDiffuse());
		pShaders["colorShader"].SetVec3("pointLights[1].specular", light.GetSpecular());
		pShaders["colorShader"].SetFloat("pointLights[1].constant", 1.0f);
		pShaders["colorShader"].SetFloat("pointLights[1].linear", 0.07f);
		pShaders["colorShader"].SetFloat("pointLights[1].quadratic", 0.3f);

		light = pLights.at(2);
		pShaders["colorShader"].SetVec3("pointLights[2].position", light.GetPosition());
		pShaders["colorShader"].SetVec3("pointLights[2].ambient", light.GetAmbient());
		pShaders["colorShader"].SetVec3("pointLights[2].diffuse", light.GetDiffuse());
		pShaders["colorShader"].SetVec3("pointLights[2].specular", light.GetSpecular());
		pShaders["colorShader"].SetFloat("pointLights[2].constant", 1.0f);
		pShaders["colorShader"].SetFloat("pointLights[2].linear", 0.07f);
		pShaders["colorShader"].SetFloat("pointLights[2].quadratic", 0.3f);

		gameObject.Draw(pShaders["colorShader"]);
	}

	//Render with swapping buffers to create bloom.
	bool horizontal = true;
	bool firstIteration = true;
	int passes = 16;
	pShaders["blurShader"].Use();

	glBindVertexArray(pQuadVAO);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pBloomFramebuffer[horizontal]);
		pShaders["blurShader"].SetInt("horizontal", horizontal);

		//On first iteration bind the hdr colorbuffer, otherwise no starting texture is provided.
		glBindTexture(GL_TEXTURE_2D, firstIteration ? pHdrColorbuffers[1] : pBloomColorbuffers[!horizontal]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;
		if (firstIteration)
		{
			firstIteration = false;
		}
	}

	//1. Render sharp bloom image into framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pHdrFramebuffer);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	pShaders["bloomShader"].Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pHdrColorbuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pBloomColorbuffers[horizontal]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);


	//2. Blur whole image
	pShaders["blurShader"].Use();
	horizontal = true;
	firstIteration = true;
	passes = 6;

	for (int i = 0; i < passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pBloomFramebuffer[horizontal]);
		pShaders["blurShader"].SetInt("horizontal", horizontal);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstIteration ? pHdrColorbuffers[1] : pBloomColorbuffers[!horizontal]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;
		if (firstIteration)
		{
			firstIteration = false;
		}
	}
	
	pShaders["dofShader"].Use();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pHdrColorbuffers[1]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pBloomColorbuffers[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pVertexPositions);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}

//This is supposed to print all sorts of general debug information.
void DebugInformation()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes is: " << nrAttributes << std::endl;
}

void CreateHDRBuffers(unsigned int& pFramebuffer, unsigned int pColorbuffers[], unsigned int& pRenderbuffer, unsigned int& pVertexPosition)
{
	//Create a framebuffer
	glGenFramebuffers(1, &pFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, pFramebuffer);

	//Create attachment for the framebuffer, which is a texture in this case, since it must be read from, otherwise a renderbuffer would be better.
	glGenTextures(2, pColorbuffers);

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, pColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		//No need to set mipmap option here.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		//Bind to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, pColorbuffers[i], 0);
	}

	//DOF======================================================================================================================
	glGenTextures(1, &pVertexPosition);
	glBindTexture(GL_TEXTURE_2D, pVertexPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Bind to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, GL_TEXTURE_2D, pVertexPosition, 0);

	//Sets framebuffer to render to two textures/outputs.
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);


	//Create Renderbuffer object for depth (and potentially stencil) values.
	glGenRenderbuffers(1, &pRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, pRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pRenderbuffer);

	//Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateBloomBuffers(unsigned int pSwappingFramebuffers[], unsigned int pSwappingColorbuffers[])
{
	glGenFramebuffers(2, pSwappingFramebuffers);

	//Create two texture for the swapping frame buffers respectively.
	glGenTextures(2, pSwappingColorbuffers);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pSwappingFramebuffers[i]);
		glBindTexture(GL_TEXTURE_2D, pSwappingColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		
		//No need to set mipmap option here.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		//Bind to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pSwappingColorbuffers[i], 0);
	}
}

void CreateRenderQuad(unsigned int& pVAO)
{
	//Create quad to render texture
	glGenVertexArrays(1, &pVAO);
	glBindVertexArray(pVAO);

	unsigned int pVBO;
	glGenBuffers(1, &pVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glBindVertexArray(0);
}
