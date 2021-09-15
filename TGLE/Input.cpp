#include "glad/glad.h"
#include "Input.h"

#include <iostream>

void HandleInput(sf::Window* pWindow, Camera *pCamera)
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
			glm::vec3 movementVector;
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
			}
			
			pCamera->Translate(movementVector * 0.1f);
			break;
		}
	}
}
