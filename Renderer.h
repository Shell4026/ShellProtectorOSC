#pragma once

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class Renderer
{
private:
	static Renderer* instance;

	sf::RenderWindow window;
	sf::Event event;
private:
	Renderer();
	~Renderer();
public:
	static auto GetInstance()->Renderer*;
	static void DestroyInstance();

	auto GetWindow()->sf::RenderWindow* const;

	void Update(const sf::Time& t);
	void Render();
};
