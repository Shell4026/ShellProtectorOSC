#include "UI.h"
#include "Renderer.h"
#include "Core.h"
#include "AutoStart.h"

#include "imgui.h"
#include "imgui-SFML.h"

UI::UI(Core& core) :
	core(core), osc(core.GetOSC())
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

	auto settingsLock = core.LockSettings();

	ImGui::SetNextWindowSize(ImVec2(Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
	ImGui::Begin("Window", 0, flags);

	float textWidth = ImGui::CalcTextSize("Shell Protector OSC 1.7").x;
	float windowWidth = ImGui::GetWindowWidth();
	float centerPosX = (windowWidth - textWidth) * 0.5f;
	ImGui::SetCursorPosX(centerPosX);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	ImGui::Text("Shell Protector OSC 1.7");
	ImGui::PopStyleColor();
	ImGui::Separator();

	ImGui::Text("Password");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(150);
	ImGui::InputText("##Password", core.password, sizeof(core.password), ImGuiInputTextFlags_Password);

	ImGui::Spacing();
	ImGui::SetNextItemWidth(50);
	ImGui::InputInt("OSC port", &core.port, 0);
	ImGui::Spacing();

	ImGui::Text("Parameter-multiplexing");
	DisplayTooltip("Only for avatars encrypted with ShellProtector 2.7 or earlier. Newer avatars are detected automatically.");
	ImGui::SameLine();
	ImGui::Checkbox("##bParameterMultiplexing", &core.bParameterMultiplexing);

	ImGui::Text("Save Option");
	ImGui::SameLine();
	ImGui::Checkbox("##save", &core.bSave);

	ImGui::Text("Start & Hide window on start");
	ImGui::SameLine();
	ImGui::Checkbox("##bStartAndHide", &core.bStartAndHide);

	ImGui::Text("Run on Windows startup");
	ImGui::SameLine();
	if (ImGui::Checkbox("##bAutoStart", &core.bAutoStart))
	{
		if (!AutoStart::SetEnabled(core.bAutoStart))
		{
			osc.AddLog("Failed to change the startup registry entry");
			core.bAutoStart = !core.bAutoStart;
		}
	}

	if (core.IsStarting())
	{
		ImGui::Text("Protected avatars found: %d", core.GetProtectedAvatarCount());
		DisplayTooltip("Avatars encrypted with a per-avatar salt. They are read from the OSC configs VRChat writes and from the list ShellProtector writes on every build, so an avatar can be tested in Gesture Manager before its first upload.");
	}

	ImGui::SetNextItemWidth(100);
	if (!core.IsStarting())
	{
		if (ImGui::Button("Start!"))
		{
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
	ImGui::SameLine();
	if (ImGui::Button("Advanced"))
		core.bShowAdvanced = true;
	ImGui::End();
}

void UI::RenderAdvanced()
{
	static ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings;

	auto settingsLock = core.LockSettings();

	ImGui::SetNextWindowSize(ImVec2(Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

	ImGui::Begin("Advanced", 0, flags);
	if (ImGui::Button("Back"))
	{
		ImGui::End();
		core.bShowAdvanced = false;
		return;
	}
	ImGui::Separator();

	ImGui::Text("OSC IP");
	DisplayTooltip("The address to send OSC data to. Change it only if VRChat runs on another device, such as a standalone Quest on the same network.");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(130);
	ImGui::InputText("##OSC IP", core.ip, sizeof(core.ip), ImGuiInputTextFlags_CharsNoBlank);
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		strcpy_s(core.ip, "127.0.0.1");

	ImGui::Text("Refresh rate(ms)");
	DisplayTooltip("The wait time before sending the next OSC data. If you can't decrypt when viewed by other users, try increasing this value.");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::InputInt("##Refresh rate", &core.refreshRate, 0, 0);

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
		osc.bLogLock = !osc.bLogLock;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	int maxLog = osc.maxLog;
	if (ImGui::InputInt("Max", &maxLog, 0, 0))
		osc.maxLog = maxLog < 1 ? 1 : maxLog;

	ImGui::Separator();
	ImGui::BeginChild("Scrolling", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_HorizontalScrollbar);

	for (auto& log : osc.GetLogs())
	{
		ImGui::TextUnformatted(log.c_str());
		ImGui::Spacing();
	}
	ImGui::EndChild();
	ImGui::End();
}