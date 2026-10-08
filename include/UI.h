#pragma once

class Core;
class OSC;

class UI
{
public:
	UI(Core& core);

	void DisplayTooltip(const char* desc);
	void Render();
	void RenderLog();
	void RenderAdvanced();
private:
	Core& core;
	OSC& osc;
};
