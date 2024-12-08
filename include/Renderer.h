#pragma once

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class Renderer
{
private:
	static inline Renderer* instance = nullptr;

	sf::RenderWindow window;
	sf::Event event;
	bool window_visibility;
public:
	static constexpr int WINDOW_WIDTH = 300;
	static constexpr int WINDOW_HEIGHT = 270;
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
