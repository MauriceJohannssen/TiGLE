#include "glad/glad.h"
#include "Input.h"

int lastMouseX = 800;
int lastMouseY = 450;
float pitch = 0.0;
float yaw = -90.0f;
bool firstMouseMovement = true;

void HandleInput(sf::Window* pWindow, Camera *pCamera, float pDeltaTime)
{
	sf::Event event;
	while(pWindow->pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			pWindow->close();
			break;

		case sf::Event::Resized:
			glViewport(0, 0, event.size.width, event.size.height);
			break;
		case sf::Event::KeyPressed:
			glm::vec3 movementVector(0);
			switch(event.key.code)
			{
			case sf::Keyboard::W:
				movementVector = pCamera->GetForward();
				break;
			case sf::Keyboard::S:
				movementVector = -pCamera->GetForward();
				break;
			case sf::Keyboard::A:
				movementVector = -glm::normalize(glm::cross(pCamera->GetForward(), pCamera->GetUp()));
				break;
			case sf::Keyboard::D:
				movementVector = glm::normalize(glm::cross(pCamera->GetForward(), pCamera->GetUp()));
				break;
			case sf::Keyboard::Escape:
				pWindow->close();
				break;
			}
			
			if(movementVector.length() > 0)
				pCamera->Translate(movementVector * 10.0f * pDeltaTime);
			break;

		case sf::Event::MouseMoved:
			
			sf::Vector2i center(pWindow->getSize().x / 2, pWindow->getSize().y / 2);
			const sf::Vector2i currentPosition = sf::Mouse::getPosition(*pWindow) - center;

			if(firstMouseMovement)
			{
				firstMouseMovement = false;
				sf::Mouse::setPosition(center, *pWindow);
			}

			sf::Mouse::setPosition(center, *pWindow);
			float mouseOffsetX = currentPosition.x;
			float mouseOffsetY = -currentPosition.y;

			float sensitivity = 15.0f;
			mouseOffsetX *= sensitivity;
			mouseOffsetY *= sensitivity;

			yaw += mouseOffsetX * pDeltaTime;
			pitch += mouseOffsetY * pDeltaTime;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			pCamera->SetForward(glm::normalize(direction));
			break;
		}
	}
}
