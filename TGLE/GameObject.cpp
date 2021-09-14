#include "GameObject.h"

unsigned int GameObject::GameObjectCount = 0;

GameObject::GameObject()
{
	//Keep track of deletions in destructor
	if(GameObjectCount == 0)
	{
		name = "gameObject_" + GameObjectCount;
	}
}

GameObject::GameObject(const std::string& name) : name(name)
{
}

GameObject::GameObject(const std::string& pName, const Material &pMaterial) : name(pName), material(pMaterial)
{
	objectSpace = glm::mat4(1);
}

void GameObject::Translate(const glm::vec3 pDirection)
{
	objectSpace = glm::translate(objectSpace, pDirection);
}

void GameObject::SetPosition(const glm::vec3 pPosition)
{
	objectSpace = glm::translate(glm::mat4(1), pPosition);
}

glm::mat4 GameObject::GetObjectMatrix() const
{
	return objectSpace;
}

