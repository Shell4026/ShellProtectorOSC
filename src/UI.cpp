#include "UI.h"
#include "Renderer.h"
#include "core.h"

#include "imgui.h"
#include "imgui-SFML.h"

UI::UI(Core& core, const Renderer& renderer) :
	core(core), renderer(renderer), osc(core.GetOSC())
{
}

void UI::DisplayTooltip(const char* desc)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(Renderer::WINDOW_WIDTH - 10);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void UI::Render()
{
	static ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowSize(ImVec2(Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
	ImGui::Begin("Window", 0, flags);

	float textWidth = ImGui::CalcTextSize("Shell Protector").x;
	float windowWidth = ImGui::GetWindowWidth();
	float centerPosX = (windowWidth - textWidth) * 0.5f;
	ImGui::SetCursorPosX(centerPosX);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	ImGui::Text("Shell Protector OSC");
	ImGui::PopStyleColor();
	ImGui::Separator();

	const static char* items[] = { "4", "8", "12", "16" };

	ImGui::Text("Password");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(150);
	ImGui::InputText("##Password", core.password, core.keyLength + 1, ImGuiInputTextFlags_Password);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::Combo(" ", &core.keyIdx, items, 4);

	core.keyLength = 4 * core.keyIdx + 4;

	ImGui::Spacing();
	ImGui::SetNextItemWidth(50);
	ImGui::InputInt("OSC port", &core.port, 0);
	ImGui::Spacing();

	ImGui::Text("Parameter-multiplexing");
	ImGui::SameLine();
	ImGui::Checkbox("##bParameterMultiplexing", &core.bParameterMultiplexing);
	if (core.bParameterMultiplexing)
	{
		ImGui::Text("Refresh rate(ms)");
		DisplayTooltip("The wait time before sending the next OSC data. If you can't decrypt when viewed by other users, try increasing this value.");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputInt("##Refresh rate", &core.refreshRate, 0, 0);
	}

	ImGui::Text("Save Option");
	ImGui::SameLine();
	ImGui::Checkbox("##save", &core.bSave);

	ImGui::Text("Start & Hide window on start");
	ImGui::SameLine();
	ImGui::Checkbox("##bStartAndHide", &core.bStartAndHide);

	ImGui::SetNextItemWidth(100);
	if (!core.IsStarting())
	{
		if (ImGui::Button("Start!"))
		{
			std::cout << core.password << '\n';
			core.StartOSC();
		}
	}
	else
	{
		if (ImGui::Button("Stop"))
			core.StopOSC();
	}

	if (ImGui::Button("Hide window"))
		core.bHideWindow = true;

	ImGui::SetCursorPosY(Renderer::WINDOW_HEIGHT - 30);
	if (ImGui::Button("Logs"))
		core.bShowLog = true;
	ImGui::End();
}

void UI::RenderLog()
{
	static ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowSize(ImVec2(Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

	ImGui::Begin("Log", 0, flags);
	if (ImGui::Button("Back"))
	{
		ImGui::End();
		core.bShowLog = false;
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		osc.ClearLogs();
	ImGui::SameLine();
	if (ImGui::Button("Lock"))
		osc.log_lock = !osc.log_lock;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Max", &osc.max_log, 0, 0);

	ImGui::Separator();
	ImGui::BeginChild("Scrolling", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_HorizontalScrollbar);

	for (auto& log : osc.GetLogs())
	{
		ImGui::Text(log.c_str());
		ImGui::Spacing();
	}
	ImGui::EndChild();
	ImGui::End();
}