#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

#include "imgui.h"
#include "imgui-SFML.h"

#include "Renderer.h"
#include "OSC.h"
#include "SHA256.h"

#include "Save.h"
#include "Load.h"

#if _WIN32
#include <windows.h>
#define MAIN APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
//#define MAIN main()
#elif
#define MAIN main()
#endif

std::atomic_bool stop = false;
std::atomic_bool start = false;

int key_idx = 0;
int key_length = 4;
char password[100];
int refreshRate = 150;

int port = 9000;

bool show_log = false;
bool parameter_multiplexing = false;
bool save_password = true;

void DisplayTooltip(const char* desc)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(290.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void DrawUI(OSC& osc)
{
	static ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowSize(ImVec2(300, 225));
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
	ImGui::InputText("##Password", password, key_length + 1, ImGuiInputTextFlags_Password);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::Combo(" ", &key_idx, items, 4);

	key_length = 4 * key_idx + 4;

	ImGui::Spacing();
	ImGui::SetNextItemWidth(50);
	ImGui::InputInt("OSC port", &port, 0);
	ImGui::Spacing();

	ImGui::Text("Parameter-multiplexing");
	ImGui::SameLine();
	ImGui::Checkbox("##parameter_multiplexing", &parameter_multiplexing);
	if (parameter_multiplexing)
	{
		ImGui::Text("Refresh rate(ms)");
		DisplayTooltip("The wait time before sending the next OSC data. If you can't decrypt when viewed by other users, try increasing this value.");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputInt("##Refresh rate", &refreshRate, 0, 0);
	}

	ImGui::Text("Save password");
	ImGui::SameLine();
	ImGui::Checkbox("##save_password", &save_password);

	ImGui::SetNextItemWidth(100);
	if (!start)
	{
		if (ImGui::Button("Start!"))
		{
			std::cout << password << '\n';
			start = true;
		}
	}
	else
	{
		if (ImGui::Button("Stop"))
			start = false;
	}

	ImGui::SetCursorPosY(225 - 30);
	if(ImGui::Button("Logs"))
	{
		show_log = true;
	}
	ImGui::End();
}
void DrawLog(OSC &osc) {
	static ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowSize(ImVec2(300, 200));
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
	
	ImGui::Begin("Log", 0, flags);
	if (ImGui::Button("Back"))
	{
		ImGui::End();
		show_log = false;
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

int MAIN
{
	std::cout << "Start...\n";
	OSC osc;
	osc.Init("127.0.0.1", port);
	std::cout << "OSC Init\n";

	Load loader;
	if (!loader.LoadFile())
		osc.AddLog("Can't load save file");
	else
	{
		for (int i = 0; i < loader.password.size(); ++i)
		{
			password[i] = loader.password[i];
		}
		key_idx = loader.key_idx;
	}
	std::cout << "Load save file\n";

	std::thread thr([&] {
		const float factor = pow(10.0f, 4);
		std::string osc_addr = "";

		auto switch0 = [&](int n) {
			bool s = false;
			osc_addr = "/avatar/parameters/encrypt_switch0";
			s = (n & 0b0001) == 1 ? true : false;
			osc.SendOSC(osc_addr, s);
		};
		auto switch1 = [&](int n) {
			bool s = false;
			osc_addr = "/avatar/parameters/encrypt_switch1";
			s = (n & 0b0010) == 2 ? true : false;
			osc.SendOSC(osc_addr, s);
		};
		auto switch2 = [&](int n) {
			bool s = false;
			osc_addr = "/avatar/parameters/encrypt_switch2";
			s = (n & 0b0100) == 4 ? true : false;
			osc.SendOSC(osc_addr, s);
		};
		auto switch3 = [&](int n) {
			bool s = false;
			osc_addr = "/avatar/parameters/encrypt_switch3";
			s = (n & 0b1000) == 8 ? true : false;
			osc.SendOSC(osc_addr, s);
		};

		while (!stop)
		{
			if (start)
			{
				osc.SetOSCPort(port);
				SHA256 sha;
				sha.update(password);
				uint8_t* digest = sha.digest();

				for (int i = 0; i < key_length; ++i)
				{
					if (parameter_multiplexing)
					{
						std::string osc_addr = "/avatar/parameters/encrypt_lock";
						osc.SendOSC(osc_addr, true);
						switch (key_length)
						{
						default: //fall down
							switch3(i);
						case 8:
							switch2(i);
						case 4:
							switch1(i);
							switch0(i);
							break;
						}
					}
					///////////////////Send password////////////////
					if (password[i] == 0)
					{
						for(int j = i + 1; j < sizeof(password); ++j)
							password[j] = 0;
					}
					float pwd;
					unsigned char var = password[i] ^ digest[i];
					osc.AddLog(std::to_string(i) + ":" + std::to_string(var));
					pwd = 1 - var / 128.0f;
					pwd = -(roundf(pwd * factor) / factor); //Rounding to 4 digits

					if(parameter_multiplexing)
						osc_addr = "/avatar/parameters/pkey";
					else
						osc_addr = "/avatar/parameters/pkey" + std::to_string(i);
					osc.SendOSC(osc_addr, pwd);
					std::this_thread::sleep_for(std::chrono::milliseconds(refreshRate));
					/////////////////////////////////////////////////
					if (parameter_multiplexing)
					{
						osc_addr = "/avatar/parameters/encrypt_lock";
						osc.SendOSC(osc_addr, false);
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});
	std::cout << "Thread Init\n";

	Renderer* renderer = Renderer::GetInstance();
	std::cout << "Renderer Init\n";
	sf::Clock delta_clock;

	while (renderer->GetWindow()->isOpen())
	{
		renderer->Update(delta_clock.restart());
		if (show_log == false)
			DrawUI(osc);
		else
			DrawLog(osc);
		renderer->Render();
	}

	stop = true;
	start = false;

	std::cout << "End\n";
	if(thr.joinable())
		thr.join();

	if (save_password)
	{
		Save saver;
		saver.password = password;
		saver.key_idx = key_idx;
		saver.SaveFile();
	}
	return 0;
}