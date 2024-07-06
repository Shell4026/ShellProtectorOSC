#include "Renderer.h"

#include "imgui.h"
#include "imgui-SFML.h"

Renderer::Renderer() :
	window(sf::VideoMode(300, 225), "ShellProtector", sf::Style::Close)
{
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
Renderer* Renderer::instance = nullptr;
