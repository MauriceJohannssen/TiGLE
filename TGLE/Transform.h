#pragma once
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

class Transform {
public:
	Transform();
	Transform(std::string& pName);
	~Transform();

	glm::vec3 GetPosition() const;
	void SetPosition(const glm::vec3 pPosition);

	void Translate(const glm::vec3 pDirection);

	glm::vec3 GetForward() const;
	void SetForward(const glm::vec3 pForward);

	glm::vec3 GetUp() const;

	void Scale(const glm::vec3 pScale);

	void Rotate(const float pAngle, const glm::vec3 pAxis);

	glm::mat4 GetObjectMatrix() const;

	std::string& GetName();

private:
	static unsigned int ObjectCount;
	std::string name;
	glm::mat4 objectSpace;
};
