#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
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
#include <map>
#include "Light.h"
#include "imgui.h"
#include "imgui-SFML.h"

//Global window settings
int windowWidth = 1600;
int windowHeight = 900;

//GUI state variables
struct GUIStateVariables {
	static bool ShowCreditsActive;
	static bool ShowDoFSettings;
	static bool ShowBloomSettings;
	static bool ShowLightingSettings;
};

bool GUIStateVariables::ShowCreditsActive = false;
bool GUIStateVariables::ShowDoFSettings = false;
bool GUIStateVariables::ShowBloomSettings = false;
bool GUIStateVariables::ShowLightingSettings = false;

//Post-processing structs
struct Bloom {
	Bloom(float threshold, int passes) : Threshold(threshold), Passes(passes) {}
	float Threshold;
	int Passes;
};

struct DepthOfField {
	DepthOfField(float aperture, float imageDistance, float planeInFocus, float near, float far) : Aperture(aperture), ImageDistance(imageDistance),
	PlaneInFocus(planeInFocus), Near(near), Far(far) {} 
	float Aperture;
	float ImageDistance;
	float PlaneInFocus;
	float Near;
	float Far;
};

//@Maurice: These vertices are being used for the rendering quad used for HDR & Bloom.
float quadVertices[] = {
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

void InitializeWindowContext(sf::RenderWindow& renderWindow);
void PrintDebugInformation();

void CreateHDRBuffers(unsigned int& framebuffer, unsigned int colorbuffers[], unsigned int& renderbuffer, unsigned int& vertexPosition);
void CreateBloomBuffers(unsigned int framebuffers[], unsigned int colorbuffers[]);
void CreateRenderQuad(unsigned int& VAO);
void LightGui(Light& light);

void Render(Camera& pCamera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix,
	std::map<std::string, Shader> pShaders, unsigned int pQuadVAO, unsigned int pHdrFramebuffer, unsigned int pHdrColorbuffers[], unsigned int pBloomFramebuffer[], unsigned int pBloomColorbuffers[],
	unsigned int& pVertexPositions, Bloom& pBloom);



int main()
{
	sf::ContextSettings contextSettings;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	contextSettings.antialiasingLevel = 4;
	contextSettings.majorVersion = 4;
	contextSettings.minorVersion = 6;

	sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), sf::String("TiGLE"), sf::Style::Default, contextSettings);

	InitializeWindowContext(window);

	PrintDebugInformation();


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
	shaders["ColorShader"] = Shader("Shaders/Lit.vert", "Shaders/Lit.frag");
	shaders["LightsShader"] = Shader("Shaders/Lit.vert", "Shaders/LightSource.frag");

	Shader textureShader("Shaders/Texture.vert", "Shaders/Texture.frag");
	textureShader.Use();
	textureShader.SetInt("screenTexture", 0);
	shaders["TextureShader"] = textureShader;

	shaders["BlurShader"] = Shader("Shaders/Texture.vert", "Shaders/GaussianBlur.frag");

	Shader bloomShader("Shaders/Texture.vert", "Shaders/BloomTonemapping.frag");
	bloomShader.Use();
	bloomShader.SetInt("hdrRender", 0);
	bloomShader.SetInt("bloomRender", 1);

	shaders["BloomShader"] = bloomShader;

	Shader DoFShader("Shaders/Texture.vert", "Shaders/DoFShader.frag");
	DoFShader.Use();
	DoFShader.SetInt("sharpTexture", 0);
	DoFShader.SetInt("blurredTexture", 1);
	DoFShader.SetInt("vertexPositions", 2);
	shaders["DofShader"] = DoFShader;

	std::vector<GameObject> gameObjects;
	std::vector<Light> lightSources;

	//GameObjects
	GameObject gameObject1("Models/R99/R99.obj");
	gameObject1.SetPosition(glm::vec3(1.5f, -0.5f, 0));
	gameObject1.Scale(glm::vec3(2));
	gameObjects.push_back(gameObject1);

	GameObject gameObject2("Models/R99/R99.obj");
	gameObject2.SetPosition(glm::vec3(-1.0f, -0.45f, -0.5));
	gameObject2.Scale(glm::vec3(2));
	gameObjects.push_back(gameObject2);

	//Lights
	Light light("light_1", LightType::Point, glm::vec3(0.1f, 0.05f, 1), "Models/Cube/Cube.obj", 6);
	light.SetPosition(glm::vec3(0.5f, 0.5f, 1));
	light.Scale(glm::vec3(0.05f));
	lightSources.push_back(light);

	Light light2("light_2", LightType::Point, glm::vec3(1, 0, 1), "Models/Cube/Cube.obj", 3);
	light2.SetPosition(glm::vec3(0.2f, 0.5, -1));
	light2.Scale(glm::vec3(0.05f));
	lightSources.push_back(light2);

	Light light3("light_3", LightType::Point, glm::vec3(0.2f, 1, 0), "Models/Cube/Cube.obj", 2);
	light3.SetPosition(glm::vec3(0.3f, 0, 0));
	light3.Scale(glm::vec3(0.05f));
	lightSources.push_back(light3);

	//Time
	sf::Clock clock;
	sf::Time deltaTime;
	sf::Time lastFrame;

	//Camera
	Camera mainCamera(ProjectionType::Perspective);
	mainCamera.SetPosition(glm::vec3(0, 0, 3));
	mainCamera.SetForward(glm::vec3(0, 0, -1));

	Bloom bloom(2.f, 20);
	DepthOfField DoF(1.f, 1.f, 1.5f, 0.1f, 2.f);

	while (window.isOpen())
	{
		//Update Time
		deltaTime = clock.getElapsedTime() - lastFrame;
		lastFrame = clock.getElapsedTime();

		//Input
		HandleInput(&window, &mainCamera, deltaTime.asSeconds());

		//Camera movement
		if (mainCamera.GetMovementVector().length() > 0.01f)
		{
			mainCamera.Translate(mainCamera.GetMovementVector() * 0.8f * deltaTime.asSeconds());
			mainCamera.SetMovementVector(mainCamera.GetMovementVector() * 0.9f);
		}

		gameObjects.at(0).Rotate(-1 * deltaTime.asSeconds(), glm::vec3(0, 1, 0));

		//Update View Matrix
		glm::mat4 view = glm::lookAt(mainCamera.GetPosition(), mainCamera.GetPosition() + mainCamera.GetForward(), mainCamera.GetUp());

		//Render
		Render(mainCamera, gameObjects, lightSources, view, mainCamera.GetProjectionMatrix(), shaders, quadVAO,
			hdrFramebuffer, hdrColorbuffers, bloomFramebuffers, bloomColorbuffers, vertexPosition, bloom);


		window.pushGLStates();

		//GUI
		ImGui::SFML::Update(window, deltaTime);

		//Menu bar
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				//Import file
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Lighting"))
		{
			if (ImGui::MenuItem("Edit Lights"))
			{
				GUIStateVariables::ShowLightingSettings = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Post Processing"))
		{
			if (ImGui::MenuItem("Depth Of Field"))
				GUIStateVariables::ShowDoFSettings = true;
			if (ImGui::MenuItem("Bloom"))
				GUIStateVariables::ShowBloomSettings = true;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About"))
		{
			if (ImGui::MenuItem("Credits"))
				GUIStateVariables::ShowCreditsActive = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		//Light Settings
		if (GUIStateVariables::ShowLightingSettings) {
			ImGui::Begin("Lighting settings", &GUIStateVariables::ShowLightingSettings);
			if (ImGui::TreeNode("Light 1")) {
				LightGui(lightSources.at(0));
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light 2")) {
				LightGui(lightSources.at(1));
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light 3")) {
				LightGui(lightSources.at(2));
				ImGui::TreePop();
			}
		}

		//Depth of Field settings
		if (GUIStateVariables::ShowDoFSettings)
		{
			ImGui::Begin("Depth of Field Settings", &GUIStateVariables::ShowDoFSettings);
			ImGui::SliderFloat("Aperture", &DoF.Aperture, 0.0f, 1.0f);
			ImGui::SliderFloat("Image Distance", &DoF.ImageDistance, 0.0f, 20.0f);
			ImGui::SliderFloat("Plane in Focus", &DoF.PlaneInFocus, 0.0f, 20.0f);
			ImGui::SliderFloat("Near", &DoF.Near, 0.0f, 20.0f);
			ImGui::SliderFloat("Far", &DoF.Far, 0.0f, 20.0f);
			ImGui::End();
		}

		//Bloom Settings
		if (GUIStateVariables::ShowBloomSettings)
		{
			ImGui::Begin("Bloom settings", &GUIStateVariables::ShowBloomSettings);
			ImGui::SliderFloat("Threshold", &bloom.Threshold, 0.0f, 10.0f);
			ImGui::SliderInt("Passes", &bloom.Passes, 2, 20);
			ImGui::End();
		}

		//Credits
		if (GUIStateVariables::ShowCreditsActive)
		{
			ImGui::Begin("Credits", &GUIStateVariables::ShowCreditsActive);
			ImGui::Text("Developed by Maurice Johannssen @ Saxion UAS");
			ImGui::End();
		}

		shaders["DofShader"].Use();
		shaders["DofShader"].SetFloat("aperture", DoF.Aperture);
		shaders["DofShader"].SetFloat("imageDistance", DoF.ImageDistance);
		shaders["DofShader"].SetFloat("planeInFocus", DoF.PlaneInFocus);
		shaders["DofShader"].SetFloat("near", DoF.Near);
		shaders["DofShader"].SetFloat("far", DoF.Far);

		//Render GUI
		ImGui::SFML::Render(window);

		//Restore previous GL states
		window.popGLStates();

		//Swap Buffers
		window.display();
		window.clear();
	}

	//Free memory
	window.close();
	ImGui::SFML::Shutdown();

	return 0;
}

//@Maurice: All engine initialization should be placed here.
void InitializeWindowContext(sf::RenderWindow& renderWindow)
{
	renderWindow.setVerticalSyncEnabled(true);
	renderWindow.setActive(true);

	sf::Image icon;
	icon.loadFromFile("icon.png");
	renderWindow.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	renderWindow.setMouseCursorVisible(false);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
	{
		std::cout << "Error: Could not initialize GLAD!";
	}

	ImGui::SFML::Init(renderWindow);

	glViewport(0, 0, windowWidth, windowHeight);
}

//@Maurice: This is supposed to print all sorts of general debug information.
void PrintDebugInformation()
{
	//Prints the amount of vertex attributes supported by the currently used OpenGL version.
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes is: " << nrAttributes << std::endl;
}


void LightGui(Light& light) {
	glm::vec3 position = light.GetPosition();
	if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f)) {
		light.SetPosition(position);
	}
	glm::vec3 lightComponent = light.GetAmbient();
	if (ImGui::ColorEdit3("Ambient", glm::value_ptr(lightComponent))) {
		light.SetAmbient(lightComponent);
	}
	lightComponent = light.GetDiffuse();
	if (ImGui::ColorEdit3("Diffuse", glm::value_ptr(lightComponent))) {
		light.SetDiffuse(lightComponent);
	}
	lightComponent = light.GetSpecular();
	if (ImGui::ColorEdit3("Specular", glm::value_ptr(lightComponent))) {
		light.SetSpecular(lightComponent);
	}
	float intensity = light.GetIntensity();
	if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 50.0f)) {
		light.SetIntensity(intensity);
	}
}

void Render(Camera& pCamera, std::vector<GameObject>& pGameObjects, std::vector<Light>& pLights, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix,
	std::map<std::string, Shader> pShaders, unsigned int pQuadVAO, unsigned int pHdrFramebuffer, unsigned int pHdrColorbuffers[], unsigned int pBloomFramebuffer[], unsigned int pBloomColorbuffers[],
	unsigned int& pVertexPositions, Bloom& pBloom)
{
	//Bind off-screen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pHdrFramebuffer);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (Light& light : pLights)
	{
		pShaders["LightsShader"].Use();
		glm::mat4 objectMatrix = light.GetObjectMatrix();
		const glm::mat4 MVPMatrix = pProjectionMatrix * pViewMatrix * objectMatrix;
		pShaders["LightsShader"].SetMat4("transform", MVPMatrix);
		pShaders["LightsShader"].SetMat4("objectMatrix", objectMatrix);
		pShaders["LightsShader"].SetVec3("lightColor", light.GetDiffuse());
		pShaders["LightsShader"].SetFloat("intensity", light.GetIntensity());
		//Bloom threshold
		pShaders["LightsShader"].SetFloat("bloomThreshold", pBloom.Threshold);
		light.Draw(pShaders["LightsShader"]);
	}


	for (GameObject& gameObject : pGameObjects)
	{
		glm::mat4 objectMatrix = gameObject.GetObjectMatrix();
		pShaders["ColorShader"].Use();
		const glm::mat4 MVPMatrix = pProjectionMatrix * pViewMatrix * objectMatrix;
		pShaders["ColorShader"].SetMat4("transform", MVPMatrix);
		pShaders["ColorShader"].SetMat4("objectMatrix", objectMatrix);
		pShaders["ColorShader"].SetVec3("cameraPosition", pCamera.GetPosition());

		//Bloom threshold
		pShaders["ColorShader"].SetFloat("bloomThreshold", pBloom.Threshold);

		//This is inefficient and just for testing purposes!
		//Use uniform buffer objects or classes.

		for (unsigned int i = 0; i < pLights.size(); i++)
		{
			Light light = pLights.at(i);
			std::string str = std::to_string(i);
			pShaders["ColorShader"].SetVec3("pointLights[" + str + "].position", light.GetPosition());
			pShaders["ColorShader"].SetVec3("pointLights[" + str + "].ambient", light.GetAmbient());
			pShaders["ColorShader"].SetVec3("pointLights[" + str + "].diffuse", light.GetDiffuse());
			pShaders["ColorShader"].SetVec3("pointLights[" + str + "].specular", light.GetSpecular());
			pShaders["ColorShader"].SetFloat("pointLights[" + str + "].constant", 1.0f);
			pShaders["ColorShader"].SetFloat("pointLights[" + str + "].linear", 0.07f);
			pShaders["ColorShader"].SetFloat("pointLights[" + str + "].quadratic", 0.3f);
			pShaders["ColorShader"].SetFloat("pointLights[" + str + "].intensity", light.GetIntensity());
		}

		gameObject.Draw(pShaders["ColorShader"]);
	}

	//Render with swapping buffers to create bloom.
	bool horizontal = true;
	bool firstIteration = true;
	pShaders["BlurShader"].Use();

	glBindVertexArray(pQuadVAO);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < pBloom.Passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pBloomFramebuffer[horizontal]);
		pShaders["BlurShader"].SetInt("horizontal", horizontal);

		//On first iteration bind the hdr colorbuffer, otherwise no starting texture is provided.
		glBindTexture(GL_TEXTURE_2D, firstIteration ? pHdrColorbuffers[1] : pBloomColorbuffers[!horizontal]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;

		if (firstIteration){
			firstIteration = false;
		}
	}

	//1. Render sharp bloom image into framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pHdrFramebuffer);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	pShaders["BloomShader"].Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pHdrColorbuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pBloomColorbuffers[horizontal]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);


	//2. Blur whole image
	pShaders["BlurShader"].Use();
	horizontal = true;
	firstIteration = true;
	int passes = 10;

	for (int i = 0; i < passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pBloomFramebuffer[horizontal]);
		pShaders["BlurShader"].SetInt("horizontal", horizontal);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstIteration ? pHdrColorbuffers[1] : pBloomColorbuffers[!horizontal]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;

		if (firstIteration){
			firstIteration = false;
		}
	}

	pShaders["DofShader"].Use();
	pShaders["DofShader"].SetVec3("cameraPosition", pCamera.GetPosition());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pHdrColorbuffers[1]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pBloomColorbuffers[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pVertexPositions);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	glBindVertexArray(0);
}

void CreateHDRBuffers(unsigned int& framebuffer, unsigned int colorbuffers[], unsigned int& renderbuffer, unsigned int& vertexPosition)
{
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(2, colorbuffers);
	for (int i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		//No need to set mipmap option here.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//Bind to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorbuffers[i], 0);
	}

	//DOF======================================================================================================================
	glGenTextures(1, &vertexPosition);
	glBindTexture(GL_TEXTURE_2D, vertexPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Bind to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, GL_TEXTURE_2D, vertexPosition, 0);

	//Sets framebuffer to render to two textures/outputs.
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//Create Renderbuffer object for depth (and potentially stencil) values.
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	//Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateBloomBuffers(unsigned int swappingFramebuffers[], unsigned int swappingColorbuffers[])
{
	glGenFramebuffers(2, swappingFramebuffers);

	//Create two texture for the swapping frame buffers respectively.
	glGenTextures(2, swappingColorbuffers);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, swappingFramebuffers[i]);
		glBindTexture(GL_TEXTURE_2D, swappingColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		//No need to set mipmap option here.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//Bind to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, swappingColorbuffers[i], 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateRenderQuad(unsigned int& VAO)
{
	//Create quad to render texture
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glBindVertexArray(0);
}
