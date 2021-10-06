#pragma once
#include "Shader.h"
#include "Transform.h"
#include <vector>
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

class GameObject : public Transform
{
public:
	GameObject(const char* pPath);
	void Draw(Shader& shader);

private:
	std::vector<Mesh> meshes;
	void LoadModel(std::string path);
	std::string directory;
	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* pScene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string& directory);
};
