#pragma once
#include <SFML/Window.hpp>
#include "Camera.h"

void HandleInput(sf::Window* pWindow, Camera* pCamera, float pDeltaTime, bool& useCSM);