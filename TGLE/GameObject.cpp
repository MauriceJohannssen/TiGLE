#include "GameObject.h"
#include <assimp/postprocess.h>
#include <stb_image.h>

std::vector<Texture> loadedTextures;

//Info::This calls transform's standard constructor, which generates a generic name.
GameObject::GameObject(const char* pPath) {
	LoadModel(pPath);
}


GameObject::GameObject(const char* pPath, std::string& pName) : Transform(pName) {
	LoadModel(pPath);
}

void GameObject::Draw(Shader& shader) {
	shader.Use();

	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}

void GameObject::LoadModel(std::string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "Error::Assimp::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));
	ProcessNode(scene->mRootNode, scene);
}

void GameObject::ProcessNode(aiNode* pNode, const aiScene* pScene) {
	for (unsigned int i = 0; i < pNode->mNumMeshes; i++) {
		aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, pScene));
	}
	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ProcessNode(pNode->mChildren[i], pScene);
	}
}

Mesh GameObject::ProcessMesh(aiMesh* mesh, const aiScene* pScene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		//Info::This parser currently only takes the first uv channel into account. Set UVs to zero
		//if non are found.
		if (mesh->mTextureCoords[0]) {
			glm::vec2 uv;
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
			vertex.UV = vector;
		}
		else {
			vertex.UV = glm::vec2(0);
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = pScene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> GameObject::LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
		aiString str;
		material->GetTexture(type, i, &str);
		bool skipTextureLoad = false;
		for (unsigned int j = 0; j < loadedTextures.size(); j++) {
			if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(loadedTextures[j]);
				skipTextureLoad = true;
				break;
			}
		}

		if (!skipTextureLoad) {
			Texture texture;
			texture.id = CreateTextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
	}

	return textures;
}

unsigned int GameObject::CreateTextureFromFile(const char* path, const std::string& directory) {
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, numberComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &numberComponents, 0);
	if (data) {
		GLenum format;
		switch (numberComponents) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			std::cout << "Error::TextureFromFile::Could not assign texture format!" << std::endl;
			return;
		}
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "Texture failed to load with path: " << path << std::endl;
	}

	stbi_image_free(data);
	return textureID;
}
