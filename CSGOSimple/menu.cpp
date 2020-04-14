#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "config.hpp"

#include "features/features.hpp"
#include "features/misc/SpecList.hpp"

//#include "lua_api/luaapi.hpp"

#include "render.hpp"
#include "globals.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;
	static int page = 1;

	static bool once{ false };
	if (!once) {
		g_Notification.Push("spectre.fun", ((std::string("Welcome back, ") + std::string(CheatGVars::UserInfo.name))).c_str());
		once = true;
	}
	g_SpecList->Draw();
	g_KeyBinds->Draw();

	CAnimations::Get().Begin(_visible);
	if (!_visible && CAnimations::Get().GetAlpha(true) <= 0.f) {
		CAnimations::Get().End();
		return;
	}

	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

	if (g_ChangeMenuSize) {
		ImGui::SetNextWindowSize(g_ChangeMenuSizeValue);
		g_ChangeMenuSize = false;
	} else {
		ImGui::SetNextWindowSizeConstraints(ImVec2(976, 450), ImVec2(4096, 4096));
	}

	Color line_color = g_Misc->colors.accent_color;
	if (CAnimations::Get().GetAlpha() < line_color.a())
		line_color.SetColor(line_color.r(), line_color.g(), line_color.b(), (int)CAnimations::Get().GetAlpha());
	ImVec4 accent = ImVec4(line_color.r() / 255.f, line_color.g() / 255.f, line_color.b() / 255.f, line_color.a() / 255.f);

	ImGui::PushStyleColor(ImGuiCol_CheckMark, accent);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent);
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, accent);
	ImGui::PushStyleColor(ImGuiCol_DragDropTarget, accent);

	if (ImGui::Begin("##spectre_win", &_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar)) {
		if (g_ShowPrompt) {
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y / 2.f);
			auto tSize = ImGui::CalcTextSize(g_PromptTitle.c_str());
			auto cSize = ImGui::GetWindowSize().x / 2.f;
			ImGui::SetCursorPosX(cSize - (tSize.x / 2.f));
			ImGui::Text(g_PromptTitle.c_str());
			ImGui::SetCursorPosX(cSize - 102);
			if (ImGui::Button("Yes", ImVec2(100, 19)) || InputSys::Get().IsKeyDown(VK_RETURN)) {
				g_PromptCallback();
				g_ShowPrompt = false;
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.f);
			if (ImGui::Button("No", ImVec2(100, 19)) || InputSys::Get().IsKeyDown(VK_ESCAPE))
				g_ShowPrompt = false;
			goto END; // ebat' kostil, ya hueyu
		}

		auto backup = ImGui::GetStyle().WindowPadding.y;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::SetCursorPos(ImVec2(0, 0));

		ImVec2 tabButtonSize = ImVec2(0, 42 + backup);
		tabButtonSize.x = (ImGui::GetCurrentWindow()->Size.x - ImGui::GetCursorPosX()) / 8;

		ImGui::SButton("spectre.fun", false, tabButtonSize);
		ImGui::SameLine();
		if (ImGui::SButton("aimbot", page == 1, tabButtonSize))
			page = 1;
		ImGui::SameLine();
		if (ImGui::SButton("triggerbot", page == 2, tabButtonSize))
			page = 2;
		ImGui::SameLine();
		if (ImGui::SButton("visuals", page == 3, tabButtonSize))
			page = 3;
		ImGui::SameLine();
		if (ImGui::SButton("misc", page == 4, tabButtonSize))
			page = 4;
		ImGui::SameLine();
		if (ImGui::SButton("config", page == 5, tabButtonSize))
			page = 5;
		ImGui::SameLine();
		if (ImGui::SButton("skins", page == 6, tabButtonSize))
			page = 6;
		ImGui::SameLine();
		if (ImGui::SButton("scripts", page == 7, tabButtonSize))
			page = 7;

		ImVec2 p = ImGui::GetCursorScreenPos();
		p.x = ImGui::GetCurrentWindow()->Pos.x - 3.f;
		p.y -= 3.202f;
		ImGui::RenderFrame(p, ImVec2(p.x + ImGui::GetWindowWidth(), p.y + 1), line_color);
		ImGui::PopStyleVar(2);

		ImGui::Dummy(ImVec2(0, 6));
		auto sz = ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - 30;
		ImGui::BeginChild("##mainwindowcontent", ImVec2(-1, sz), false, ImGuiWindowFlags_DoNotFillBG); {
			ImGui::Dummy(ImVec2(9, 0));
			switch (page) {
			case 1:
				 g_Aimbot.MenuAimbot();
				break;
			case 2:
				g_Aimbot.MenuTrigger();
				break;
			case 3:
				g_Visuals->Menu();
				break;
			case 4:
				g_Misc->Menu();
				break;
			case 5:
				g_Config->Menu();
				break;
			case 6:
				g_SkinChanger->Menu();
				break;
			case 7:
				//g_LuaApiManager->Menu();
				ImGui::Text("hhh");
				break;
			}
		}
		ImGui::EndChild();

#ifdef _STABLE
		ImGui::Text("[stable]");
#elif _BETA
		ImGui::Text("[beta]");
#elif _DEBUG
		ImGui::Text("[dev]");
#endif

		std::string rightsidetext;
		rightsidetext += "logged in as ";
		rightsidetext += CheatGVars::UserInfo.name;
		rightsidetext += ", days left: ";
		rightsidetext += std::to_string(CheatGVars::UserInfo.days_left);

		if (g_LocalPlayer && g_EngineClient && g_EngineClient->IsInGame() && g_EngineClient->IsConnected() && g_LocalPlayer->m_hActiveWeapon()) {
			short defindex = g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
			std::string gname = g_CustomWeaponGroups->GetGroupName(defindex);
			if (gname.empty()) {
				ImGui::SameLine();
				ImGui::Text("   [!] Weapon not found in any groups");
			}
		}


		auto rightsidetext_size = ImGui::CalcTextSize(rightsidetext.c_str());
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x - rightsidetext_size.x - 10.f);
		ImGui::Text(rightsidetext.c_str());
	}
END:
	g_MenuSize = ImGui::GetCurrentWindow()->Size;
	ImGui::End();

	CAnimations::Get().End();

	ImGui::PopStyleColor(4);
}

void Menu::Toggle()
{
	_visible = !_visible;
	
	if (_visible) {
		AimbotFirstTimeRender = true;
		TriggerFirstTimeRender = true;
		SkinsFirstTimeRender = true;
	}
}

void Menu::CreateStyle()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	ImGui::GetStyle().WindowRounding = 0.f;
	ImGui::GetIO().LogFilename = NULL;
	ImGui::GetIO().IniFilename = NULL;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(33 / 255.f, 35 / 255.f, 47 / 255.f, 1.0f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(30 / 255.f, 30 / 255.f, 41 / 255.f, 1.0f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ChildWindowBg] = ImVec4(33 / 255.f, 35 / 255.f, 47 / 255.f, 1.0f);
	colors[ImGuiCol_FrameBg] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.08f, 0.08f, 0.08f, 1.f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f);
	colors[ImGuiCol_TitleBg] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f); //
	colors[ImGuiCol_ButtonActive] = ImVec4(135 / 255.f, 135 / 255.f, 135 / 255.f, 1.0f); //
	colors[ImGuiCol_Header] = ImVec4(167 / 255.f, 24 / 255.f, 71 / 255.f, 1.0f); //multicombo, combo selected item color.
	colors[ImGuiCol_HeaderHovered] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f);
	colors[ImGuiCol_HeaderActive] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f);
	colors[ImGuiCol_Separator] = ImVec4(1.f, 1.f, 1.f, 1.f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(1.f, 1.f, 1.f, 1.f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1.f, 1.f, 1.f, 1.f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_CheckMark] = ImVec4(167 / 255.f, 24 / 255.f, 71 / 255.f, 1.0f);
}

