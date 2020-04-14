#pragma once
#include <Windows.h>
#include "moveax/moveax.hpp"

namespace CheatGVars {
	extern float OFOV;
	extern int SWidth;
	extern int SHeight;
	extern int SWidthHalf;
	extern int SHeightHalf;
	extern bool SlowWalking;
	extern bool UpdateNightMode;
	extern LPVOID lpvReserved;
	extern moveax_user_info_t UserInfo;
}