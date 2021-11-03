#include "Transform.h"

unsigned int Transform::ObjectCount = 0;

Transform::Transform() : objectSpace(glm::mat4(1))
{
	//TODO: Keep track of deletions in destructor
	if (ObjectCount == 0)
	{
		name = "gameObject_" + ObjectCount++;
	}
}

Transform::Transform(std::string& pName) : name(pName), objectSpace(glm::mat4(1))
{
	
}

glm::vec3 Transform::GetPosition() const
{
	return objectSpace[3];
}

void Transform::SetPosition(const glm::vec3 pPosition)
{	
	objectSpace[3] = glm::vec4(pPosition,1);
 	//objectSpace = glm::translate(glm::mat4(1), pPosition);
}

void Transform::Translate(const glm::vec3 pDirection)
{
	//objectSpace = glm::translate(objectSpace, pDirection);
	//y tho?
	objectSpace[3] += glm::vec4(pDirection, 0);
}

glm::vec3 Transform::GetForward() const
{
	return objectSpace[2];
}

void Transform::SetForward(const glm::vec3 pForward)
{
	objectSpace[2] = glm::vec4(pForward, 0);
}

glm::vec3 Transform::GetUp() const
{
	return objectSpace[1];
}

void Transform::Scale(const glm::vec3 pScale)
{
	objectSpace = glm::scale(objectSpace, pScale);
}


glm::mat4* Transform::GetObjectMatrix() const
{
	return new glm::mat4(objectSpace);
}
