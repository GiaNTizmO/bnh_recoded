#pragma once
#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../misc/misc.hpp"


class CResolver {
private:
	int missedshots = 0;
public:
	void LegitResolver();
};

extern CResolver* g_Resolver;