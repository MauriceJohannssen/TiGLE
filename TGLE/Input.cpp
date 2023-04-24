#include "glad/glad.h"
#include "Input.h"
#include "imgui.h"
#include "imgui-SFML.h"

int lastMouseX = 800;
int lastMouseY = 450;
float pitch = 0.0;
float yaw = -90.0f;
bool firstMouseMovement = true;

void HandleInput(sf::Window* window, Camera* camera, float deltaTime, bool& useCSM) {
	sf::Event event;
	while (window->pollEvent(event)) {
		ImGui::SFML::ProcessEvent(event);

		switch (event.type) {
		case sf::Event::Closed:
			window->close();
			break;

		case sf::Event::Resized:
			glViewport(0, 0, event.size.width, event.size.height);
			break;

		case sf::Event::KeyPressed:
			switch (event.key.code) {
			case sf::Keyboard::W:
				camera->SetMovementVector(camera->GetMovementVector() + camera->GetForward());
				break;
			case sf::Keyboard::S:
				camera->SetMovementVector(camera->GetMovementVector() - camera->GetForward());
				break;
			case sf::Keyboard::A:
				camera->SetMovementVector(camera->GetMovementVector() - glm::normalize(glm::cross(camera->GetForward(), camera->GetUp())));
				break;
			case sf::Keyboard::D:
				camera->SetMovementVector(camera->GetMovementVector() + glm::normalize(glm::cross(camera->GetForward(), camera->GetUp())));
				break;
			case sf::Keyboard::Escape:
				window->close();
				break;
			case sf::Keyboard::P:
				useCSM = !useCSM;
				break;
			}

		case sf::Event::MouseMoved:
			if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
				break;
			}

			sf::Vector2i center(window->getSize().x / 2, window->getSize().y / 2);
			const sf::Vector2i currentPosition = sf::Mouse::getPosition(*window) - center;

			if (firstMouseMovement) {
				firstMouseMovement = false;
				sf::Mouse::setPosition(center, *window);
			}

			sf::Mouse::setPosition(center, *window);
			float mouseOffsetX = static_cast<float>(currentPosition.x);
			float mouseOffsetY = static_cast<float>( - currentPosition.y);

			float sensitivity = 15.0f;
			mouseOffsetX *= sensitivity;
			mouseOffsetY *= sensitivity;

			yaw += mouseOffsetX * deltaTime;
			pitch += mouseOffsetY * deltaTime;

			if (pitch > 90.0f)
				pitch = 90.0f;
			if (pitch < -90.0f)
				pitch = -90.0f;

			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			camera->SetForward(glm::normalize(direction));
			break;
		}
	}
}
