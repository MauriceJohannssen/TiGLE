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
	window.setVerticalSyncEnabled(false);
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

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	window.close();

	return 0;
}