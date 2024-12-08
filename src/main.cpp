#include "PCH.h"
#include "Core.h"
#include "Renderer.h"
#include "UI.h"

#include <iostream>
#include <string>

#if _WIN32
#include <windows.h>
#define MAIN APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#else
#define MAIN main()
#endif

int MAIN
{
	Core core;
	core.Init();

	Renderer* renderer = Renderer::GetInstance();
	std::cout << "Renderer Init\n";

	UI ui{ core, *renderer };

	sf::Clock deltaClock;

	if (core.IsHideWindow())
		renderer->HideWindow();
	else
		renderer->ShowWindow();

	while (renderer->GetWindow()->isOpen())
	{
		renderer->Update(deltaClock.restart());
		if (!core.IsShowLog())
			ui.Render();
		else
			ui.RenderLog();
		renderer->Render();

		if (core.bHideWindow)
			renderer->HideWindow();
		else
			renderer->ShowWindow();

		if (core.IsFinish())
			renderer->GetWindow()->close();
	}
	if (!core.IsFinish())
		core.Shutdown();
	return 0;
}