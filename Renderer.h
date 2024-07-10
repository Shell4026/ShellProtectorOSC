#pragma once

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

const int WINDOW_WIDTH = 300;
const int WINDOW_HEIGHT = 270;

class Renderer
{
private:
	static Renderer* instance;

	sf::RenderWindow window;
	sf::Event event;
	bool window_visibility;
private:
	Renderer();
	~Renderer();
public:
	static auto GetInstance()->Renderer*;
	static void DestroyInstance();

	auto GetWindow()->sf::RenderWindow* const;

	void Update(const sf::Time& t);
	void Render();

	void ShowWindow();
	void HideWindow();
	void Stop();
};
