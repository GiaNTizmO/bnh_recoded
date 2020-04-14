#define NOMINMAX
#include <Windows.h>
#include "features/features.hpp"
#include "features/visuals/chams.hpp"

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"

//#include "lua_api/luaapi.hpp"

#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"
#include "globals.hpp"

#include "moveax/moveax.hpp"
/*#ifndef _DEBUG
    #pragma comment(lib, "moveax_public.lib")
#endif*/

class c_hack : public i_moveax_hack_listener {
public:
    void on_handshake(bool success, moveax_user_info_t& user_info) override {
        if (!success) {
            ExitProcess(0);
            return;
        }

        CheatGVars::UserInfo = user_info;

        //char buffer[128] = { 0 };
        //snprintf(buffer, sizeof(buffer),
        //    "Welcome back, %s!\nDays left: %d",
        //    user_info.name, user_info.days_left
        //);
        //MessageBoxA(0, buffer, "bruh", 0);
    }
};

c_hack hack;

DWORD WINAPI OnDllAttach(LPVOID base)
{
	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(1000);

    Beep(408, 257);
//#ifdef _DEBUG
    moveax_user_info_t user_info = { "user", 1337 };
    hack.on_handshake(true, user_info);
//#else
//    moveax_public::init(CheatGVars::lpvReserved, &hack);
//
//    while (moveax_public::is_connected())
//        moveax_public::run();
//#endif

#ifdef _DEBUG
    Utils::AttachConsole();
#endif

    //try {
        Utils::ConsolePrint("Initializing...\n");

        Interfaces::Initialize();
        Interfaces::Dump();

        NetvarSys::Get().Initialize();
        InputSys::Get().Initialize();
		Render::Get().Initialize();
#ifndef _DEBUG
        game_data::initialize_kits();
#endif
        Menu::Get().Initialize();
        g_Chams = new Chams();
        g_Config->SetupValues();
        g_Config->refresh();
        Hooks::Initialize();

        //g_LuaApiManager->Initialize();
        //g_LuaApiManager->Refresh();

        // Register some hotkeys.
        // - Note:  The function that is called when the hotkey is pressed
        //          is called from the WndProc thread, not this thread.
        // 

        // Panic button
        //InputSys::Get().RegisterHotkey(VK_DELETE, [base]() {
        //    g_Unload = true;
        //});

        // Menu Toggle
        InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
            Menu::Get().Toggle();
        });

        Utils::ConsolePrint("Finished.\n");
		Utils::ConsolePrint("Built on: %s %s\n", __DATE__, __TIME__);
        g_CVar->FindVar("crosshair")->SetValue(true);

        while(!g_Unload)
            Sleep(1000);

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);

    //} catch(const std::exception& ex) {
    //    Utils::ConsolePrint("An error occured during initialization:\n");
    //    Utils::ConsolePrint("%s\n", ex.what());
    //    Utils::ConsolePrint("Press any key to exit.\n");
    //    Utils::ConsoleReadKey();
    //    Utils::DetachConsole();

    //    FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    //}

    // unreachable
    //return TRUE;
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Utils::DetachConsole();
#endif

    Hooks::Shutdown();

    Menu::Get().Shutdown();
    return TRUE;
}

BOOL WINAPI DllMain(
    _In_      HINSTANCE hinstDll,
    _In_      DWORD     fdwReason,
    _In_opt_  LPVOID    lpvReserved
)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            CheatGVars::lpvReserved = lpvReserved;
            DisableThreadLibraryCalls(hinstDll);
            CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
            return TRUE;
        case DLL_PROCESS_DETACH:
            if(lpvReserved == nullptr)
                return OnDllDetach();
            return TRUE;
        default:
            return TRUE;
    }
}
