#pragma once
#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../../config.hpp"

class SpecList {
private:
	struct speclist_cfg_t {
		bool enabled = false;
		int x = 100;
		int y = 100;
	};
public:
	speclist_cfg_t params;
	void Draw();
	void SetupValues();
};

extern SpecList* g_SpecList;