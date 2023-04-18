#pragma once
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

class Transform {
public:
	Transform();
	Transform(std::string& pName);
	Transform(const Transform& other);
	~Transform();

	glm::vec3 GetPosition() const;
	void SetPosition(const glm::vec3 pPosition);

	void Translate(const glm::vec3 pDirection);

	glm::vec3 GetForward() const;
	void SetForward(const glm::vec3 pForward);

	glm::vec3 GetUp() const;

	void Scale(const glm::vec3 pScale);

	void Rotate(const float pAngle, const glm::vec3 pAxis);

	void LookAt(const glm::vec3 pos);

	glm::mat4 GetObjectMatrix() const;

	std::string& GetName();


	Transform* parent;
private:
	static unsigned int ObjectCount;
	std::string name;
	glm::mat4 objectSpace;
};
