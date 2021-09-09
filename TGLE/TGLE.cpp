#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <iostream>
#include "Input.h"

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 3;
	settings.minorVersion = 3;

	sf::Window window(sf::VideoMode(1600, 900), "TGLE", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(true);
	window.setActive(true);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
	{
		std::cout << "Could not initialize GLAD";
	}

	glViewport(0, 0, 1600, 900);
	while (window.isOpen())
	{
		HandleInput(&window);

		//Draw

		window.display();
	}

	window.close();

	return 0;
}