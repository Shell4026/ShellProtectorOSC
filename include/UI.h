#pragma once

class Core;
class Renderer;
class OSC;

class UI
{
private:
	Core& core;
	const Renderer& renderer;
	OSC& osc;
public:
	UI(Core& core, const Renderer& renderer);

	void DisplayTooltip(const char* desc);
	void Render();
	void RenderLog();
};