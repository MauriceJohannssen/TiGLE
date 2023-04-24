#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include "Input.h"
//#include "Material.h"
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
#include <ctime>
#include <cstdlib>
#include "Scene.h"
#include <chrono>

//Global window settings
constexpr int windowWidth = 1600;
constexpr int windowHeight = 900;

const unsigned int ShadowWidth = 2048;
const unsigned int ShadowHeight = ShadowWidth;

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

//Post-processing
struct Bloom {
	Bloom() : IsEnabled(false), Threshold(0.0f), Passes(2) {}
	Bloom(float threshold, int passes, bool enabled = false) : IsEnabled(enabled), Threshold(threshold), Passes(passes) {}
	bool IsEnabled;
	float Threshold;
	int Passes;

	void Render() {}
};

struct DepthOfField {
	DepthOfField() : IsEnabled(false), Aperture(0.5f), ImageDistance(1.0f), PlaneInFocus(1.0f), Near(1.0f), Far(1.0f) {}
	DepthOfField(float aperture, float imageDistance, float planeInFocus, float near, float far, bool enabled = false)
		: IsEnabled(enabled), Aperture(aperture), ImageDistance(imageDistance),
		PlaneInFocus(planeInFocus), Near(near), Far(far) {}

	bool IsEnabled;
	float Aperture;
	float ImageDistance;
	float PlaneInFocus;
	float Near;
	float Far;

	void SetupDof() {}

	void Render(Shader& shader, glm::vec3 cameraPos, unsigned int sharpColorBuffer, unsigned int blurredColorBuffer, unsigned int vertexPosTex)
	{
		shader.Use();
		shader.SetVec3("cameraPosition", cameraPos);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sharpColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, blurredColorBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, vertexPosTex);
	}
};

class PostProcessingEffects
{
public:
	PostProcessingEffects(Bloom bloom, DepthOfField dof) : Bloom(bloom), DepthOfField(dof) {

	}

	Bloom& GetBloom() {
		return Bloom;
	}

	DepthOfField& GetDoF() {
		return DepthOfField;
	}

private:
	Bloom Bloom;
	DepthOfField DepthOfField;
};

//These vertices are being used for the rendering quad used for HDR & Bloom.
float quadVertices[] = {
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

//=====================================================================================================================

void InitializeWindowContext(sf::RenderWindow& renderWindow);
void PrintDebugInformation();

void CreateHDRBuffers(unsigned int& framebuffer, unsigned int colorbuffers[], unsigned int& renderbuffer, unsigned int& vertexPosition);
void CreateBloomBuffers(unsigned int framebuffers[], unsigned int colorbuffers[]);
void CreateRenderQuad(unsigned int& VAO);
void LightGui(Light& light);
void RenderDof(std::map<std::string, Shader>& shaders, Camera& camera, unsigned int sharpColorBuffer, unsigned int blurredColorBuffer, unsigned int vertexPositionTex);

void Render(Scene& scene, const glm::mat4& viewMatrix,
	std::map<std::string, Shader> shaders, unsigned int quadVAO, unsigned int HDRFramebuffer, unsigned int HDRColorbuffers[], unsigned int bloomFramebuffers[], unsigned int bloomColorbuffers[],
	unsigned int& vertexPositions, PostProcessingEffects& postProcessingEffects, unsigned int shadowBuffer, unsigned int depthTex, unsigned int csmBuffer, unsigned int csmTexts, unsigned int csmUBO, const std::vector<float>& cascadedShadowLevels);


std::vector<glm::mat4> GetLightSpaceMatrices(Camera& camera, glm::vec3 lightDirection, const std::vector<float>& cascadedShadowLevels);

int main()
{
	sf::ContextSettings contextSettings;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	contextSettings.antialiasingLevel = 8;
	contextSettings.majorVersion = 4;
	contextSettings.minorVersion = 6;

	sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), sf::String("TiGLE"), sf::Style::Default, contextSettings);

	InitializeWindowContext(window);

	PrintDebugInformation();

	srand(time(nullptr));

	std::ofstream file("FrametimeCSM.csv");

	// Check if the file was opened successfully
	if (!file.is_open()) {
		std::cout << "Failed to open file!" << std::endl;
		return 1;
	}
	else {
		std::cout << "CSV file open" << std::endl;
	}

	Scene scene;

	//Camera
	scene.GetMainCamera().SetPosition(glm::vec3(2, 1, 0.2f));
	scene.GetMainCamera().SetForward(glm::vec3(1, 0, 0));

	//Setup shadow=====================================================================================================
	unsigned int shadowMapFBO;
	glGenFramebuffers(1, &shadowMapFBO);

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowWidth, ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Setup CSM========================================================================================================

	float cameraFarPlane = std::get<1>(scene.GetMainCamera().GetNearFarPlanes());
	std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };

	unsigned int CSM_FBO;
	glGenFramebuffers(1, &CSM_FBO);

	unsigned int CSM_DepthMaps;
	glGenTextures(1, &CSM_DepthMaps);
	glBindTexture(GL_TEXTURE_2D_ARRAY, CSM_DepthMaps);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, ShadowWidth, ShadowHeight,
		int(shadowCascadeLevels.size()) + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

	glBindFramebuffer(GL_FRAMEBUFFER, CSM_FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CSM_DepthMaps, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int matricesUBO;
	glGenBuffers(1, &matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//=================================================================================================================
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

	//Shaders
	std::map<std::string, Shader> shaders;
	shaders["ColorShader"] = Shader("Shaders/Lit.vert", "Shaders/Lit.frag");
	shaders["LightsShader"] = Shader("Shaders/Lit.vert", "Shaders/LightSource.frag");

	//Shadowmapping
	shaders["DepthShader"] = Shader("Shaders/DepthShader.vert", "Shaders/DepthShader.frag");
	shaders["CSMDepthShader"] = Shader("Shaders/CSM_DepthShader.vert", "Shaders/CSM_DepthShader.frag", "Shaders/CSM_DepthShader.geom");

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

	//GameObjects
	GameObject gameObject1("Models/R99/R99.obj");
	gameObject1.SetPosition(glm::vec3(1.5f, -0.5f, 0));
	gameObject1.Scale(glm::vec3(2));
	scene.GetGameObjects().push_back(gameObject1);

	GameObject ground("Models/Cube/Cube.obj");
	ground.Scale(glm::vec3(50, 0.01f, 12));
	ground.SetPosition(glm::vec3(40, -1.0f, 0));
	scene.GetGameObjects().push_back(ground);

	float z = 7.0f;
	for (unsigned row = 0; row < 2; ++row) {
		for (unsigned int i = 0; i < 10; ++i) {
			GameObject house("Models/House/House.obj");
			house.SetPosition(glm::vec3(8.0f * i, -1.0f, z));
			float randRot = row * 110.0f + rand() % 5;
			house.Rotate(randRot, glm::vec3(0, 1, 0));
			scene.GetGameObjects().push_back(house);

			GameObject lamp("Models/Lamp/Lamp.obj");
			lamp.SetPosition(glm::vec3(5 * i, -1.0, 0));
			scene.GetGameObjects().push_back(lamp);
		}

		z *= -1;
	}

	//Lights
	Light sun("sun", LightType::Directional, glm::vec3(1, 1, 1), "Models/Cube/Cube.obj", 2.0f);
	sun.SetPosition(glm::vec3(-15, 15, 0));
	sun.SetForward(glm::normalize(glm::vec3(1, -1, 1)));
	sun.Scale(glm::vec3(0.1f));
	scene.GetLights().push_back(sun);

	Light light("light_1", LightType::Point, glm::vec3(0.1f, 0.05f, 1), "Models/Cube/Cube.obj", 30);
	light.SetPosition(glm::vec3(50.0f, 8.0f, 0.0f));
	light.Scale(glm::vec3(0.05f));
	scene.GetLights().push_back(light);

	Light light2("light_2", LightType::Point, glm::vec3(1, 0, 1), "Models/Cube/Cube.obj", 25);
	light2.SetPosition(glm::vec3(25.0f, 4.0f, 0.0f));
	light2.Scale(glm::vec3(0.05f));
	scene.GetLights().push_back(light2);

	Light light3("light_3", LightType::Point, glm::vec3(0.7f, 0.2f, 0.7f), "Models/Cube/Cube.obj", 10);
	light3.SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
	light3.Scale(glm::vec3(0.05f));
	scene.GetLights().push_back(light3);

	//Time
	sf::Clock clock;
	sf::Time deltaTime;
	sf::Time lastFrame;

	PostProcessingEffects postProcessingEffects(Bloom(2.0f, 20), DepthOfField(1.0f, 1.0f, 1.5f, 0.1f, 2.0f));

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "OpenGL vendor: " << glGetString(GL_VENDOR) << std::endl;

	//Main Engine Loop=================================================================================================
	while (window.isOpen())
	{
		//Update Time
		deltaTime = clock.getElapsedTime() - lastFrame;
		lastFrame = clock.getElapsedTime();

		auto startTime = std::chrono::high_resolution_clock::now();

		Camera& mainCamera = scene.GetMainCamera();

		//Input
		HandleInput(&window, &mainCamera, deltaTime.asSeconds());

		////Camera movement
		//if (mainCamera.GetMovementVector().length() > 0.01f)
		//{
		//	mainCamera.Translate(mainCamera.GetMovementVector() * 0.8f * deltaTime.asSeconds());
		//	mainCamera.SetMovementVector(mainCamera.GetMovementVector() * 0.9f);
		//}

		mainCamera.Translate(glm::vec3(1, 0, 0) * 0.8f * deltaTime.asSeconds());

		//Update View Matrix
		glm::mat4 view = glm::lookAt(mainCamera.GetPosition(), mainCamera.GetPosition() + mainCamera.GetForward(), mainCamera.GetUp());

		//Render
		Render(scene, view, shaders, quadVAO, hdrFramebuffer, hdrColorbuffers, bloomFramebuffers,
			bloomColorbuffers, vertexPosition, postProcessingEffects, shadowMapFBO, depthMap, CSM_FBO, CSM_DepthMaps, matricesUBO, shadowCascadeLevels);

		//Save GL states before ImGui draw
		window.pushGLStates();

		//GUI==========================================================================================================
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

		ImGui::Begin("Scene Hierarchy");
		for (GameObject& gameObject : scene.GetGameObjects())
		{
			ImGui::Text("%s", gameObject.GetName().c_str());
		}

		for (Light& light : scene.GetLights())
		{
			ImGui::Text("%s", light.GetName().c_str());
		}
		ImGui::End();

		//Light Settings
		auto& lightSources = scene.GetLights();
		if (GUIStateVariables::ShowLightingSettings) {
			ImGui::Begin("Lighting settings", &GUIStateVariables::ShowLightingSettings);
			if (ImGui::TreeNode("Light 1")) {
				LightGui(lightSources.at(1));
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light 2")) {
				LightGui(lightSources.at(2));
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light 3")) {
				LightGui(lightSources.at(3));
				ImGui::TreePop();
			}
		}

		//Depth of Field settings
		if (GUIStateVariables::ShowDoFSettings)
		{
			DepthOfField& DoF = postProcessingEffects.GetDoF();
			ImGui::Begin("Depth of Field Settings", &GUIStateVariables::ShowDoFSettings);
			ImGui::Checkbox("Enabled", &DoF.IsEnabled);
			ImGui::SliderFloat("Aperture", &DoF.Aperture, 0.0f, 1.0f);
			ImGui::SliderFloat("Image Distance", &DoF.ImageDistance, 0.0f, 20.0f);
			ImGui::SliderFloat("Plane in Focus", &DoF.PlaneInFocus, 0.0f, 20.0f);
			ImGui::SliderFloat("Near", &DoF.Near, 0.0f, 20.0f);
			if (DoF.Near >= DoF.Far)
			{
				DoF.Far = DoF.Near + 0.01f;
			}
			ImGui::SliderFloat("Far", &DoF.Far, 0.0f, 20.0f);
			ImGui::End();
		}

		//Bloom Settings
		if (GUIStateVariables::ShowBloomSettings)
		{
			Bloom& bloom = postProcessingEffects.GetBloom();
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

		DepthOfField& DoF = postProcessingEffects.GetDoF();
		shaders["DofShader"].Use();
		shaders["DofShader"].SetFloat("aperture", DoF.Aperture);
		shaders["DofShader"].SetFloat("imageDistance", DoF.ImageDistance);
		shaders["DofShader"].SetFloat("planeInFocus", DoF.PlaneInFocus);
		shaders["DofShader"].SetFloat("near", DoF.Near);
		shaders["DofShader"].SetFloat("far", DoF.Far);


		//Render GUI
		ImGui::SFML::Render(window);

		//Restore previous GL states after ImGui draw
		window.popGLStates();

		//Swap Buffers
		window.display();
		window.clear();

		auto endTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

		float frameTime = static_cast<float>(elapsedTime) / 1000.0f; // convert to ms

		//std::cout << "Frame time: " << frameTime << "s" << std::endl;
		//std::cout << "Frame rate: " << fps << "fps" << std::endl;
		//static int counter = 0;
		//float fps = 1000.0f / frameTime;
		//file << counter++ << "," << frameTime << std::endl;
	}

	file.close();

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
	glEnable(GL_MULTISAMPLE);
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

void RenderLights(Shader& shader, std::vector<Light> lights,
	const glm::mat4 MVMatrix, PostProcessingEffects postProcessingEffects)
{

	for (Light& light : lights)
	{
		shader.Use();
		glm::mat4 objectMatrix = light.GetObjectMatrix();
		const glm::mat4 MVPMatrix = MVMatrix * objectMatrix;
		shader.SetMat4("transform", MVPMatrix);
		shader.SetMat4("objectMatrix", objectMatrix);
		shader.SetVec3("lightColor", light.GetDiffuse());
		shader.SetFloat("intensity", light.GetIntensity());

		//Bloom threshold
		shader.SetFloat("bloomThreshold", postProcessingEffects.GetBloom().Threshold);
		light.Draw(shader);
	}
}

void RenderObjects(Scene& scene, Shader& shader, const glm::mat4 MVMatrix, const glm::mat4 lightSpaceMatrix,
	unsigned int& depthMap, PostProcessingEffects postProcessingEffects, unsigned int& lightDepthMaps, const std::vector<float>& cascadedShadowLevels)
{
	for (GameObject& gameObject : scene.GetGameObjects())
	{
		glm::mat4 objectMatrix = gameObject.GetObjectMatrix();
		shader.Use();
		const glm::mat4 MVPMatrix = MVMatrix * objectMatrix;
		shader.SetMat4("transform", MVPMatrix);
		shader.SetMat4("objectMatrix", objectMatrix); //Doubled. Bad.
		shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		shader.SetVec3("cameraPosition", scene.GetMainCamera().GetPosition());

		//Bloom threshold
		shader.SetFloat("bloomThreshold", postProcessingEffects.GetBloom().Threshold);

		shader.SetInt("shadowMap", 15);
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		shader.SetInt("cascadedShadowMaps", 10);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);


		shader.SetFloat("farPlane", std::get<1>(scene.GetMainCamera().GetNearFarPlanes()));
		shader.SetInt("cascadeCount", cascadedShadowLevels.size());
		for (size_t i = 0; i < cascadedShadowLevels.size(); ++i)
		{
			shader.SetFloat("cascadePlaneDistances[" + std::to_string(i) + "]", cascadedShadowLevels[i]);
		}

		shader.SetMat4("viewMatrix", MVMatrix);

		//This is inefficient and just for testing purposes!
		//Use uniform buffer objects or classes.
		for (unsigned int i = 0; i < scene.GetLights().size(); i++)
		{
			Light light = scene.GetLights()[i];
			std::string str = std::to_string(i - 1);

			switch (light.GetLightType()) {
			case LightType::Point:
				shader.SetVec3("pointLights[" + str + "].position", light.GetPosition());
				shader.SetVec3("pointLights[" + str + "].ambient", light.GetAmbient());
				shader.SetVec3("pointLights[" + str + "].diffuse", light.GetDiffuse());
				shader.SetVec3("pointLights[" + str + "].specular", light.GetSpecular());
				shader.SetFloat("pointLights[" + str + "].constant", 1.0f);
				shader.SetFloat("pointLights[" + str + "].linear", 0.07f);
				shader.SetFloat("pointLights[" + str + "].quadratic", 0.3f);
				shader.SetFloat("pointLights[" + str + "].intensity", light.GetIntensity());
				break;


			case LightType::Directional:
				shader.SetVec3("directionalLight.direction", light.GetForward());
				shader.SetVec3("directionalLight.ambient", light.GetAmbient());
				shader.SetVec3("directionalLight.diffuse", light.GetDiffuse());
				shader.SetVec3("directionalLight.specular", light.GetSpecular());
				shader.SetFloat("directionalLight.intensity", light.GetIntensity());
				break;

			default:
				std::cout << "Hit default case for light type." << std::endl;
				break;
			}
		}

		gameObject.Draw(shader);
	}
}

void Render(Scene& scene, const glm::mat4& viewMatrix,
	std::map<std::string, Shader> shaders, unsigned int quadVAO, unsigned int HDRFramebuffer, unsigned int HDRColorbuffers[], unsigned int bloomFramebuffers[], unsigned int bloomColorbuffers[],
	unsigned int& vertexPositions, PostProcessingEffects& postProcessingEffects, unsigned int shadowBuffer, unsigned int depthTex, unsigned int csmBuffer, unsigned int csmTexts, unsigned int csmUBO, const std::vector<float>& cascadedShadowLevels)
{
	//Depth Buffer=====================================================================================================
	auto cameraPlanes = scene.GetMainCamera().GetNearFarPlanes();
	glm::mat4 lightProjection = glm::ortho(-50.f, 50.f, -50.f, 50.f, std::get<0>(cameraPlanes), std::get<1>(cameraPlanes));
	glm::mat4 lightView = glm::lookAt(glm::vec3(-1.5f, 3, -3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	glViewport(0, 0, ShadowWidth, ShadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_CULL_FACE); //Peter Panning
	//glCullFace(GL_FRONT);

	for (GameObject& gameObject : scene.GetGameObjects())
	{
		glm::mat4 objectMatrix = gameObject.GetObjectMatrix();
		shaders["DepthShader"].Use();
		shaders["DepthShader"].SetMat4("model", objectMatrix);
		shaders["DepthShader"].SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		gameObject.Draw(shaders["DepthShader"]);
	}

	//glCullFace(GL_BACK);
	glViewport(0, 0, windowWidth, windowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//CSM==============================================================================================================
	const auto lightMatrices = GetLightSpaceMatrices(scene.GetMainCamera(), scene.GetLights()[0].GetForward(), cascadedShadowLevels);

	glBindBuffer(GL_UNIFORM_BUFFER, csmUBO);
	for (unsigned int i = 0; i < lightMatrices.size(); ++i)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, ShadowWidth, ShadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, csmBuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_CLAMP);
	//glEnable(GL_CULL_FACE); //Peter Panning
	//glCullFace(GL_FRONT);

	for (GameObject& gameObject : scene.GetGameObjects())
	{
		shaders["CSMDepthShader"].Use();
		shaders["CSMDepthShader"].SetMat4("model", gameObject.GetObjectMatrix());
		gameObject.Draw(shaders["CSMDepthShader"]);
	}

	//glCullFace(GL_BACK);
	//glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_CLAMP);
	glViewport(0, 0, windowWidth, windowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//=================================================================================================================

	//Bind off-screen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, HDRFramebuffer);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto projMatrix = scene.GetMainCamera().GetProjectionMatrix();
	RenderLights(shaders["LightsShader"], scene.GetLights(), projMatrix * viewMatrix, postProcessingEffects);
	RenderObjects(scene, shaders["ColorShader"], projMatrix * viewMatrix, lightSpaceMatrix, depthTex, postProcessingEffects, csmTexts, cascadedShadowLevels);

	//Render with swapping buffers to create bloom.
	bool horizontal = true;
	bool firstIteration = true;
	shaders["BlurShader"].Use();

	glBindVertexArray(quadVAO);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < postProcessingEffects.GetBloom().Passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffers[horizontal]);
		shaders["BlurShader"].SetInt("horizontal", horizontal);

		//On first iteration bind the HDR colorbuffer, otherwise no starting texture is provided.
		glBindTexture(GL_TEXTURE_2D, firstIteration ? HDRColorbuffers[1] : bloomColorbuffers[!horizontal]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;

		if (firstIteration) {
			firstIteration = false;
		}
	}

	//1. Render sharp bloom image into framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, HDRFramebuffer);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	shaders["BloomShader"].Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HDRColorbuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomColorbuffers[horizontal]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);


	//2. Blur whole image
	shaders["BlurShader"].Use();
	horizontal = true;
	firstIteration = true;

	for (int i = 0; i < postProcessingEffects.GetBloom().Passes; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffers[horizontal]);
		shaders["BlurShader"].SetInt("horizontal", horizontal);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstIteration ? HDRColorbuffers[1] : bloomColorbuffers[!horizontal]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

		horizontal = !horizontal;

		if (firstIteration) {
			firstIteration = false;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (postProcessingEffects.GetDoF().IsEnabled)
	{
		postProcessingEffects.GetDoF().Render(shaders["DofShader"],
			scene.GetMainCamera().GetPosition(), HDRColorbuffers[1], bloomColorbuffers[0], vertexPositions);
	}
	else
	{
		shaders["TextureShader"].Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, HDRColorbuffers[1]);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D_ARRAY, csmTexts);
	}

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

	//DOF==============================================================================================================
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

std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
	const auto inverse = glm::inverse(projectionMatrix * viewMatrix);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x) {
		for (unsigned int y = 0; y < 2; ++y) {
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 point = inverse * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(point / point.w);
			}
		}
	}

	return frustumCorners;
}

glm::mat4 GetLightMatrix(const float nearPlane, const float farPlane, Camera& camera, glm::vec3 lightDirection)
{
	const auto proj = glm::perspective(glm::radians(90.0f),
		(float)windowWidth / (float)windowHeight, nearPlane, farPlane);

	const auto view = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp());

	const auto corners = GetFrustumCornersWorldSpace(proj, view);

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& corner : corners) {
		center += glm::vec3(corner);
	}

	center /= corners.size();

	const auto lightView = glm::lookAt(center - lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));


	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (const auto& v : corners) {
		const auto transfPoint = lightView * v;
		minX = std::min(minX, transfPoint.x);
		maxX = std::max(maxX, transfPoint.x);
		minY = std::min(minY, transfPoint.y);
		maxY = std::max(maxY, transfPoint.y);
		minZ = std::min(minZ, transfPoint.z);
		maxZ = std::max(maxZ, transfPoint.z);
	}

	constexpr float zAxisMultiplier = 10.0f;

	if (minZ < 0) {
		minZ *= zAxisMultiplier;
	}
	else {
		minZ /= zAxisMultiplier;
	}

	if (maxZ < 0) {
		maxZ /= zAxisMultiplier;
	}
	else {
		maxZ *= zAxisMultiplier;
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

	return lightProjection * lightView;
}

std::vector<glm::mat4> GetLightSpaceMatrices(Camera& camera, glm::vec3 lightDirection, const std::vector<float>& cascadedShadowLevels) {
	std::vector<glm::mat4> matrices;

	for (unsigned int shadowLvl = 0; shadowLvl < cascadedShadowLevels.size() + 1; ++shadowLvl) {
		if (shadowLvl == 0) {
			matrices.push_back(GetLightMatrix(std::get<0>(camera.GetNearFarPlanes()), cascadedShadowLevels[shadowLvl], camera, lightDirection));
		}
		else if (shadowLvl < cascadedShadowLevels.size()) {
			matrices.push_back(GetLightMatrix(cascadedShadowLevels[shadowLvl - 1], cascadedShadowLevels[shadowLvl], camera, lightDirection));
		}
		else {
			matrices.push_back(GetLightMatrix(cascadedShadowLevels[shadowLvl - 1], std::get<1>(camera.GetNearFarPlanes()), camera, lightDirection));
		}
	}

	return matrices;
}
