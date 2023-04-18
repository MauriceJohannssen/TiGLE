#include "Transform.h"

unsigned int Transform::ObjectCount = 0;

Transform::Transform() : objectSpace(glm::mat4(1)) {
	//TODO: This could create naming conflicts.
	name = "GameObject_" + std::to_string(ObjectCount++);
}

Transform::Transform(std::string& pName) : name(pName), objectSpace(glm::mat4(1)) {
}

Transform::Transform(const Transform& other) {
	objectSpace = other.objectSpace;
	name = other.name;
}

Transform::~Transform() {
}

glm::vec3 Transform::GetPosition() const {
	return objectSpace[3];
}

void Transform::SetPosition(const glm::vec3 pPosition) {
	objectSpace[3] = glm::vec4(pPosition, 1);
}

void Transform::Translate(const glm::vec3 pDirection) {
	objectSpace[3] += glm::vec4(pDirection, 0);
}

glm::vec3 Transform::GetForward() const {
	return objectSpace[2];
}

void Transform::SetForward(const glm::vec3 pForward) {
	objectSpace[2] = glm::vec4(pForward, 0);
}

glm::vec3 Transform::GetUp() const {
	return objectSpace[1];
}

void Transform::Scale(const glm::vec3 pScale) {
	objectSpace = glm::scale(objectSpace, pScale);
}

void Transform::Rotate(const float pAngle, const glm::vec3 pAxis) {
	objectSpace = glm::rotate(objectSpace, pAngle, pAxis);
}

void Transform::LookAt(const glm::vec3 pos) {
	glm::vec3 direction = glm::normalize(pos - GetPosition());
	glm::mat4 newRot = glm::lookAt(GetPosition(), direction, glm::vec3(0,1,0));
	objectSpace = glm::translate(glm::mat4(1.0f), GetPosition()) * newRot;
}

glm::mat4 Transform::GetObjectMatrix() const {
	return objectSpace;
}

std::string& Transform::GetName()	{
	return name;
}