#pragma once
#include "glad/glad.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	Shader();
	Shader(const Shader& pShader);

	unsigned int ID;

	void Use() const;
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetVec3(const std::string& name, glm::vec3 value) const;
	void SetMat4(const std::string& name, glm::mat4 value) const;
};
