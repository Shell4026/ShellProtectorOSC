#include "PCH.h"
#include "Renderer.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>

Renderer::Renderer()
{
	window.create(sf::VideoMode(1, 1), "ShellProtector", sf::Style::Close);
	window.setVisible(false);
	window_visibility = false;
	std::cout << "Window Init\n";
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
}

Renderer::~Renderer()
{
	ImGui::SFML::Shutdown();
}

auto Renderer::GetInstance() -> Renderer*
{
	if (instance == nullptr)
		instance = new Renderer;
	return instance;
}

void Renderer::DestroyInstance()
{
	if (instance != nullptr)
		delete instance;
}

auto Renderer::GetWindow() -> sf::RenderWindow* const
{
	return &window;
}

void Renderer::Update(const sf::Time& t)
{
	while (window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(window, event);
		if (event.type == sf::Event::Closed)
		{
			window.close();
		}
	}
	if (window.hasFocus())
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
			window.close();
	}
	ImGui::SFML::Update(window, t);
}
void Renderer::Render()
{
	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}

void Renderer::ShowWindow()
{
	if (window_visibility) return;
	window.setVisible(true);
	window_visibility = true;
	window.setSize(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT));
}

void Renderer::HideWindow()
{
	if (!window_visibility) return;
	window.setVisible(false);
	window_visibility = false;
	window.setSize(sf::Vector2u(1, 1));
}

void Renderer::Stop() {
	window.close();
}