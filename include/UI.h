#pragma once

class Core;
class OSC;

class UI
{
private:
	Core& core;
	OSC& osc;
public:
	UI(Core& core);

	void DisplayTooltip(const char* desc);
	void Render();
	void RenderLog();
};