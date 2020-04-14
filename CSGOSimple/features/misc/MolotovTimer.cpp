#include "MolotovTimer.hpp"
#include "../../helpers/utils.hpp"
#include "../../helpers/math.hpp"
#include "../../render.hpp"
#include "misc.hpp"

MolotovTimer g_MolotovTimer;

void MolotovTimer::Draw() {
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
		return;

	for (auto i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)
	{
		auto ent = C_BaseEntity::GetEntityByIndex(i);
		if (!ent)
			continue;

		if (ent->GetClientClass()->m_ClassID != ClassId_CInferno)
			continue;

		auto inferno = reinterpret_cast<Inferno_t*>(ent);

		auto origin = inferno->m_vecOrigin();
		auto screen_origin = Vector();

		if (!Math::WorldToScreen(origin, screen_origin))
			return;

		const auto spawn_time = inferno->GetSpawnTime();
		const auto timer = (spawn_time + Inferno_t::GetExpireTime()) - g_GlobalVars->curtime;
		const auto factor = timer / Inferno_t::GetExpireTime();
		const auto l_spawn_time = *(float*)(uintptr_t(inferno) + 0x20);
		const auto l_factor = ((l_spawn_time + 7.03125f) - g_GlobalVars->curtime) / 7.03125f;
		
		//static auto size = ImVec2(60, 15);

		//Render::Get().RenderBoxFilled(screen_origin.x - size.x, screen_origin.y - size.y, screen_origin.x, screen_origin.y, Color::Black);
		//Render::Get().RenderBoxFilled(screen_origin.x, screen_origin.y, screen_origin.x - 4 * factor, screen_origin.y, Color::White);
		////Render::Get().RenderBoxFilled(screen_origin.x - size.x + 2, screen_origin.y - size.y + 2, ((screen_origin.x - 2 * factor)), screen_origin.y - 2, Color::White);
		Render::Get().RenderBoxFilled(screen_origin.x - 49, screen_origin.y + 10, (screen_origin.x - 49) + 98.f, (screen_origin.y + 10) + 4.f, Color::Black);
		Render::Get().RenderBoxFilled(screen_origin.x - 49, screen_origin.y + 10, (screen_origin.x - 49) + (98.f * l_factor), (screen_origin.y + 10) + 4.f, Color::Red);

		Render::Get().RenderText(std::to_string((int)(round(timer + 1))) + "s", screen_origin.x, screen_origin.y - 5.f, 18.f, g_Misc->colors.molotov_timer_color);
		Render::Get().RenderCircle3D(origin, 50, 150.f, g_Misc->colors.molotov_timer_color);


		//Render::Get().RenderBoxFilled(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, (double)screen_origin.x, (double)screen_origin.y, Color::Black);
		//Render::Get().RenderBoxFilled(screen_origin.x - size.x * 0.5 + 2, screen_origin.y - size.y * 0.5 + 2, (double)screen_origin.x - 4 * factor, (double)screen_origin.y - 4, g_Misc->colors.accent_color);

		//Render::Get().RenderBox(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, (double)size.x, (double)size.y, Color::White);
	}
}
