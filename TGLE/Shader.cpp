#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::ifstream shaderFilePath;

	shaderFilePath.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	std::string vertexCode;
	std::string fragmentCode;

	try
	{
		std::stringstream shaderStream1, shaderStream2;
		
		//Get vertex shader
		shaderFilePath.open(vertexPath);
		shaderStream1 << shaderFilePath.rdbuf();
		vertexCode = shaderStream1.str();
		shaderFilePath.close();
		shaderFilePath.clear();

		//Get fragment shader
		shaderFilePath.open(fragmentPath);
		shaderStream2 << shaderFilePath.rdbuf();
		fragmentCode = shaderStream2.str();
		shaderFilePath.close();
		shaderFilePath.clear();
	
	}
	catch (std::ifstream::failure exception)
	{
		std::cout << "SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const char* vertexShaderCode = vertexCode.c_str();
	const char* fragmentShaderCode = fragmentCode.c_str();

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
	glCompileShader(fragmentShader);

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Use() const
{
	glUseProgram(ID);
}


void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

