#pragma once

#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/vec3.hpp>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath); 
	unsigned int ID;
	void Use() const;
	void SetBool(const std::string &name, bool value) const;
	void SetInt(const std::string &name, int value) const;
	void SetFloat(const std::string &name, float value) const;
	void SetVec3(const std::string& name, glm::vec3 value) const;
};

#endif