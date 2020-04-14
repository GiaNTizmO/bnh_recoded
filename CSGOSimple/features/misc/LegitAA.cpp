#include "LegitAA.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/input.hpp"
#include "../../render.hpp"
#include "misc.hpp"

LegitAA* g_LegitAA = new LegitAA();

void LegitAA::CreateMove(CUserCmd* pCmd, bool* bSendPacket) {
	if (pCmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_USE) ||
		g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP ||
		!g_LocalPlayer->IsAlive())
		return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon) 
		return;

	short weapon_idx = weapon->m_Item().m_iItemDefinitionIndex();
	if ((weapon_idx == WEAPON_GLOCK || weapon_idx == WEAPON_FAMAS) && weapon->m_flNextPrimaryAttack() >= g_GlobalVars->curtime)
		return;


	if (weapon_idx == WEAPON_HEGRENADE ||
		weapon_idx == WEAPON_FRAG_GRENADE ||
		weapon_idx == WEAPON_INCGRENADE ||
		weapon_idx == WEAPON_SMOKEGRENADE ||
		weapon_idx == WEAPON_TAGRENADE ||
		weapon_idx == WEAPON_DECOY ||
		weapon_idx == WEAPON_FLASHBANG ||
		weapon_idx == WEAPON_MOLOTOV) { // hadrcoded aswell
		if (!weapon->m_bPinPulled() && weapon->m_fThrowTime() > 0.f)
			return;

		if (((pCmd->buttons & IN_ATTACK) || (pCmd->buttons & IN_ATTACK2)) && weapon->m_fThrowTime() > 0.f)
			return;
	}

	static float SpawnTime = 0.0f;
	if (g_LocalPlayer->m_flSpawnTime() != SpawnTime) {
		AnimState.pBaseEntity = g_LocalPlayer;
		g_LocalPlayer->ResetAnimationState(&AnimState);
		SpawnTime = g_LocalPlayer->m_flSpawnTime();
	}


	QAngle OldAngles = pCmd->viewangles;

	static bool broke_lby = false;
	bool change_side = false;

	if (g_Misc->options.legit_aa_flip) {
		change_side = true;
		g_Misc->options.legit_aa_flip = false;
	}
		//side = -side;

	float minimal_move;
	bool should_move;
	switch (g_Misc->options.legit_aa_mode) {
	case 0: // static
		if (change_side)
			side = side;
		minimal_move = 2.f;
		if (g_LocalPlayer->m_fFlags() & FL_DUCKING)
			minimal_move *= 3.f;
		if (pCmd->buttons & IN_WALK)
			minimal_move *= 3.f;
		should_move = g_LocalPlayer->m_vecVelocity().Length2D() <= 0.f || std::fabsf(g_LocalPlayer->m_vecVelocity().z) <= 100.f;
		if (pCmd->command_number % 2 == 1) {
			pCmd->viewangles.yaw += 120.f * side;
			if (should_move)
				pCmd->sidemove -= minimal_move;
			*bSendPacket = false;
		}
		else if (should_move)
			pCmd->sidemove += minimal_move;
		break;
	case 1: // balanced (lby)
		if (change_side)
			side = -side;
		if (next_lby >= g_GlobalVars->curtime) {
			if (!broke_lby && *bSendPacket && g_ClientState->chokedcommands > 0)
				return;

			broke_lby = false;
			*bSendPacket = false;
			pCmd->viewangles.yaw += 120.0f * side;
		}
		else {
			broke_lby = true;
			*bSendPacket = false;
			pCmd->viewangles.yaw += 120.0f * -side;
		}
		break;
	case 2: // balanced (micomovements)
		if (change_side)
			side = -side;
		pCmd->viewangles.yaw += *bSendPacket ? 0.f : g_LocalPlayer->GetMaxDesyncDelta() * side;

		if (g_LocalPlayer->m_vecVelocity().Length2D() == 0 || g_LocalPlayer->m_vecVelocity().z == 0)
			pCmd->sidemove = pCmd->tick_count % 2 ? 2.f : -2.f; // alternates every other tick so we're not just walking, we're going left and right and generally not moving too far
		break;
	case 3: // test mode
		if (!change_side) {
			if (g_ClientState->chokedcommands <= 8) {
				*bSendPacket = false;
				pCmd->viewangles.yaw += 232;
			} else {
				*bSendPacket = true;
				pCmd->viewangles.yaw -= 5;
			}
		} else {
			pCmd->viewangles.yaw -= 2;
			if (g_ClientState->chokedcommands <= 8) {
				*bSendPacket = false;
				pCmd->viewangles.yaw -= 225;
			} else {
				*bSendPacket = true;
			}
		}
		break;
	}

	Math::FixAngles(pCmd->viewangles);
	Math::MovementFix(pCmd, OldAngles, pCmd->viewangles);

	pCmd->viewangles.yaw = std::remainderf(pCmd->viewangles.yaw, 360.f);

	static int latency_ticks = 0;
	float fl_latency = g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
	int latency = TIME_TO_TICKS(fl_latency);;
	if (g_ClientState->chokedcommands <= 0) {
		latency_ticks = latency;
	}
	else {
		latency_ticks = max(latency, latency_ticks);
	}

	int max_choke_ticks = 14;
	if (g_GameRules->m_bIsValveDS()) {
		if (fl_latency >= g_GlobalVars->interval_per_tick)
			max_choke_ticks = 11 - latency_ticks;
		else
			max_choke_ticks = 11;
	}
	else {
		max_choke_ticks = 13 - latency_ticks;
	}
	if (g_ClientState->chokedcommands >= max_choke_ticks) {
		*bSendPacket = true;
		pCmd->viewangles = g_ClientState->viewangles;
	}

	auto anim_state = g_LocalPlayer->GetPlayerAnimState();
	if (anim_state) {
		CCSGOPlayerAnimState anim_state_backup = *anim_state;
		*anim_state = AnimState;
		*g_LocalPlayer->GetVAngles() = pCmd->viewangles;
		g_LocalPlayer->UpdateClientSideAnimation();

		if (anim_state->speed_2d > 0.1f || std::fabsf(anim_state->flUpVelocity)) {
			next_lby = g_GlobalVars->curtime + 0.22f;
		}
		else if (g_GlobalVars->curtime > next_lby) {
			if (std::fabsf(Math::AngleDiff(anim_state->m_flGoalFeetYaw, anim_state->m_flEyeYaw)) > 35.0f) {
				next_lby = g_GlobalVars->curtime + 1.1f;
			}
		}

		AnimState = *anim_state;
		*anim_state = anim_state_backup;
	}

	if (*bSendPacket) {
		real_angle = AnimState.m_flGoalFeetYaw;
		view_angle = AnimState.m_flEyeYaw;
	}
}

void LegitAA::DrawArrows() {
	if (!g_Misc->options.legit_aa) 
		return;

	auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr) {
		Vector forward;
		Math::AngleVectors(QAngle(0.0f, angle, 0.0f), forward);
		float AngleLinesLength = (float)g_Misc->options.aa_arrows_length;

		Vector w2sReal;
		if (Math::WorldToScreen(origin + forward * AngleLinesLength, w2sReal) && w2sOrigin.x > 0.f && w2sOrigin.y > 0) {
			Render::Get().RenderLine(w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, Color::White, 1.0f);
			Render::Get().RenderText(text, w2sReal.x, w2sReal.y + 10.0f, 14.0f, clr, true, true);
		}
	};

	if (g_Misc->options.aa_arrows) {
		Vector w2sOrigin;
		if (Math::WorldToScreen(g_LocalPlayer->m_vecOrigin(), w2sOrigin)) {
			drawAngleLine(g_LocalPlayer->m_vecOrigin(), w2sOrigin, view_angle, "viewangles", Color(0.937f, 0.713f, 0.094f, 1.0f));
			drawAngleLine(g_LocalPlayer->m_vecOrigin(), w2sOrigin, g_LocalPlayer->m_flLowerBodyYawTarget(), "lby", Color(0.0f, 0.0f, 1.0f, 1.0f));
			drawAngleLine(g_LocalPlayer->m_vecOrigin(), w2sOrigin, real_angle, "real", Color(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}
}