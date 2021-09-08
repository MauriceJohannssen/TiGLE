#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	
	sf::Window window(sf::VideoMode(800, 600), "TGLE", sf::Style::Close, settings);
	window.setVerticalSyncEnabled(true);
	window.setActive(true);
	bool running = true;
	while (running)
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				running = false;
				break;

			case sf::Event::Resized:
				glViewport(0, 0, event.size.width, event.size.height);
				break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw

		window.display();
	}

	return 0;
}