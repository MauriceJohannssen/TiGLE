#include "Scene.h"

Scene::Scene() : mainCamera(ProjectionType::Perspective){

}

 std::vector<GameObject>& Scene::GetGameObjects() {
	return gameObjects;
}

 std::vector<Light>& Scene::GetLights()  {
	return lights;
}

 Camera& Scene::GetMainCamera(){
	return mainCamera;
}
