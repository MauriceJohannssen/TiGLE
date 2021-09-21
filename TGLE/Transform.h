#pragma once
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

class Transform
{
public:
	Transform();
	Transform(std::string& pName);

	//Transform related methods
	glm::vec3 GetPosition();
	void SetPosition(const glm::vec3 pPosition);

	void Translate(const glm::vec3 pDirection);
	
	glm::vec3 GetForward();
	void SetForward(const glm::vec3 pForward);
	
	glm::vec3 GetUp();

	void Scale(glm::vec3 pScale);
	
	glm::mat4* GetObjectMatrix() const;

private:
	static unsigned int ObjectCount;
	std::string name;
	glm::mat4 objectSpace;
};
