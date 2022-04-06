#pragma once
#include "Shader.h"
#include "Transform.h"
#include <vector>
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

class GameObject : public Transform {
public:
	GameObject(const char* pPath);
	GameObject(const char* pPath, std::string& pName);
	void Draw(Shader& shader);

private:
	void LoadModel(std::string path);
	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* pScene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
	static unsigned int CreateTextureFromFile(const char* path, const std::string& directory);
	std::string directory;
	std::vector<Mesh> meshes;
};
