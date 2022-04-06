#include "glad/glad.h"
#include "Input.h"
#include "imgui.h"
#include "imgui-SFML.h"

int lastMouseX = 800;
int lastMouseY = 450;
float pitch = 0.0;
float yaw = -90.0f;
bool firstMouseMovement = true;

void HandleInput(sf::Window* pWindow, Camera* pCamera, float pDeltaTime) {
	sf::Event event;
	while (pWindow->pollEvent(event)) {
		ImGui::SFML::ProcessEvent(event);

		switch (event.type) {
		case sf::Event::Closed:
			pWindow->close();
			break;

		case sf::Event::Resized:
			glViewport(0, 0, event.size.width, event.size.height);
			break;

		case sf::Event::KeyPressed:
			switch (event.key.code) {
			case sf::Keyboard::W:
				pCamera->SetMovementVector(pCamera->GetMovementVector() + pCamera->GetForward());
				break;
			case sf::Keyboard::S:
				pCamera->SetMovementVector(pCamera->GetMovementVector() - pCamera->GetForward());
				break;
			case sf::Keyboard::A:
				pCamera->SetMovementVector(pCamera->GetMovementVector() - glm::normalize(glm::cross(pCamera->GetForward(), pCamera->GetUp())));
				break;
			case sf::Keyboard::D:
				pCamera->SetMovementVector(pCamera->GetMovementVector() + glm::normalize(glm::cross(pCamera->GetForward(), pCamera->GetUp())));
				break;
			case sf::Keyboard::Escape:
				pWindow->close();
				break;
			}
			break;

		case sf::Event::MouseMoved:
			if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
				break;
			sf::Vector2i center(pWindow->getSize().x / 2, pWindow->getSize().y / 2);
			const sf::Vector2i currentPosition = sf::Mouse::getPosition(*pWindow) - center;

			if (firstMouseMovement) {
				firstMouseMovement = false;
				sf::Mouse::setPosition(center, *pWindow);
			}

			sf::Mouse::setPosition(center, *pWindow);
			float mouseOffsetX = static_cast<float>(currentPosition.x);
			float mouseOffsetY = static_cast<float>( - currentPosition.y);

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
