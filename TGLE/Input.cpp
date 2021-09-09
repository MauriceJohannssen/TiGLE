#include "glad/glad.h"
#include "Input.h"

void HandleInput(sf::Window* window)
{
	sf::Event event;
	while(window->pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			window->close();
			break;

		case sf::Event::Resized:
			glViewport(0, 0, event.size.width, event.size.height);
			break;
		}
	}
}
