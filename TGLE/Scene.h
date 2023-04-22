#pragma once
#include <vector>
#include "GameObject.h"
#include "Light.h"
#include "Camera.h"

class Scene
{
public:
	Scene();
	 std::vector<GameObject>& GetGameObjects();
	 std::vector<Light>& GetLights();
	 Camera& GetMainCamera();

private:
	std::vector<GameObject> gameObjects;
	std::vector<Light> lights;
	Camera mainCamera;
};

