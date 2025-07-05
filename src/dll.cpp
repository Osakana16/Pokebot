module;
#include <vector>


module pokebot: dll;
import :plugin;

import pokebot.bot.behavior;
import pokebot.game;
import pokebot.game.util;
import pokebot.util.tracer;
import pokebot.util.random;
import pokebot.terrain.graph;

enginefuncs_t g_engfuncs;
globalvars_t* gpGlobals;

// START of Metamod stuff

enginefuncs_t meta_engfuncs, meta_engfuncs_post;
gamedll_funcs_t* gpGamedllFuncs;
mutil_funcs_t* gpMetaUtilFuncs;
meta_globals_t* gpMetaGlobals;

DLL_FUNCTIONS func_table;
DLL_FUNCTIONS gFunctionTable_Post;

META_FUNCTIONS gMetaFunctionTable{
    nullptr, // pfnGetEntityAPI()
    nullptr, // pfnGetEntityAPI_Post()
    GetEntityAPI2, // pfnGetEntityAPI2()
    GetEntityAPI2_Post, // pfnGetEntityAPI2_Post()
    nullptr, // pfnGetNewDLLFunctions()
    nullptr, // pfnGetNewDLLFunctions_Post()
    GetEngineFunctions, // pfnGetEngineFunctions()
    nullptr,            // pfnGetEngineFunctions_Post()
};

plugin_info_t Plugin_info{
    const_cast<char*>(META_INTERFACE_VERSION),  // interface version
    const_cast<char*>("Pokebot"),               // plugin name
    const_cast<char*>("Test Version"),          // plugin version
    const_cast<char*>(__DATE__),                // date of creation
    const_cast<char*>(""),                      // plugin author
    const_cast<char*>(""),                      // plugin URL
    const_cast<char*>("POKEBOT"),               // plugin logtag
    PT_CHANGELEVEL,                             // when loadable
    PT_ANYTIME,                                 // when unloadable
};

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            pokebot::plugin::Pokebot::OnDllAttached();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

C_DLLEXPORT int Meta_Query(char* ifvers, plugin_info_t** pPlugInfo, mutil_funcs_t* pMetaUtilFuncs) {
    // this function is the first function ever called by metamod in the plugin DLL. Its purpose
    // is for metamod to retrieve basic information about the plugin, such as its meta-interface
    // version, for ensuring compatibility with the current version of the running metamod.

    // keep track of the pointers to metamod function tables metamod gives us
    gpMetaUtilFuncs = pMetaUtilFuncs;
    *pPlugInfo = &Plugin_info;

    // check for interface version compatibility
    if (strcmp(ifvers, Plugin_info.ifvers) != 0) {
        int mmajor = 0, mminor = 0, pmajor = 0, pminor = 0;

        LOG_CONSOLE(PLID, "%s: meta-interface version mismatch (metamod: %s, %s: %s)", Plugin_info.name, ifvers, Plugin_info.name, Plugin_info.ifvers);
        LOG_MESSAGE(PLID, "%s: meta-interface version mismatch (metamod: %s, %s: %s)", Plugin_info.name, ifvers, Plugin_info.name, Plugin_info.ifvers);

        // if plugin has later interface version, it's incompatible (update metamod)
        sscanf(ifvers, "%d:%d", &mmajor, &mminor);
        sscanf(META_INTERFACE_VERSION, "%d:%d", &pmajor, &pminor);

        if ((pmajor > mmajor) || ((pmajor == mmajor) && (pminor > mminor))) {
            LOG_CONSOLE(PLID, "metamod version is too old for this plugin; update metamod");
            LOG_ERROR(PLID, "metamod version is too old for this plugin; update metamod");
            return (FALSE);
        }

        // if plugin has older major interface version, it's incompatible (update plugin)
        else if (pmajor < mmajor) {
            LOG_CONSOLE(PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
            LOG_ERROR(PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
            return (FALSE);
        }
    }
    return (TRUE); // tell metamod this plugin looks safe
}

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME now, META_FUNCTIONS* pFunctionTable, meta_globals_t* pMGlobals, gamedll_funcs_t* pGamedllFuncs) {
    // this function is called when metamod attempts to load the plugin. Since it's the place
    // where we can tell if the plugin will be allowed to run or not, we wait until here to make
    // our initialization stuff, like registering CVARs and dedicated server commands.

    // are we allowed to load this plugin now ?
    if (now > Plugin_info.loadable) {
        LOG_CONSOLE(PLID, "%s: plugin NOT attaching (can't load plugin right now)", Plugin_info.name);
        LOG_ERROR(PLID, "%s: plugin NOT attaching (can't load plugin right now)", Plugin_info.name);
        return (FALSE); // returning FALSE prevents metamod from attaching this plugin
    }

    // keep track of the pointers to engine function tables metamod gives us
    gpMetaGlobals = pMGlobals;
    memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
    gpGamedllFuncs = pGamedllFuncs;

    pokebot::bot::behavior::DefineBehavior();

    pokebot::plugin::Pokebot::RegisterCommand();

    // print a message to notify about plugin attaching
    LOG_CONSOLE(PLID, "%s: plugin attaching", Plugin_info.name);
    LOG_MESSAGE(PLID, "%s: plugin attaching", Plugin_info.name);

    // ask the engine to register the server commands this plugin uses
    return (TRUE); // returning TRUE enables metamod to attach this plugin
}


C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason) {
    // this function is called when metamod unloads the plugin. A basic check is made in order
    // to prevent unloading the plugin if its processing should not be interrupted.

    // is metamod allowed to unload the plugin ?
    if ((now > Plugin_info.unloadable) && (reason != PNL_CMD_FORCED)) {
        LOG_CONSOLE(PLID, "%s: plugin NOT detaching (can't unload plugin right now)", Plugin_info.name);
        LOG_ERROR(PLID, "%s: plugin NOT detaching (can't unload plugin right now)", Plugin_info.name);
        return (FALSE); // returning FALSE prevents metamod from unloading this plugin
    }
    return (TRUE); // returning TRUE enables metamod to unload this plugin
}

edict_t* worldspawn = NULL;

C_DLLEXPORT void WINAPI
GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals) {
    // get the engine functions from the engine...
    memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
    gpGlobals = pGlobals;
}

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion) {
    func_table.pfnStartFrame = []() -> void {
        using namespace pokebot;

        plugin::Pokebot::OnUpdate();
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnGameInit = []() -> void {
		pokebot::plugin::Pokebot::OnGameInit();
        // pokebot::ParseWeaponJson();
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnSpawn = [](edict_t* entity) -> int {
		pokebot::plugin::Pokebot::OnEntitySpawned(entity);
        pokebot::plugin::Pokebot::AppendSpawnedEntity(entity);
		pokebot::plugin::Pokebot::OnEntitySpawned();

        RETURN_META_VALUE(MRES_IGNORED, 0);
    };

    func_table.pfnClientConnect = [](edict_t* entity, const char* Name, const char* Address, char Reject_Reason[]) -> qboolean {
        if (gpGlobals->deathmatch) {
            // check if this client is the listen server client
            if (strcmp(Address, "loopback") == 0) {
                SERVER_PRINT(std::format("POKEBOT: The host is connected.\n", STRING(entity->v.netname)).c_str());
                // save the edict of the listen server client...
                pokebot::game::game.host.SetHost(entity);
                // REMOVED: We cannot get the player's name at this timing.
                // pokebot::game::game.clients.Register(entity);

                pokebot::plugin::Pokebot::OnMapLoaded();
            }
            pokebot::plugin::Pokebot::OnClientConnect(entity, Address);
            SERVER_PRINT(std::format("POKEBOT: {} is connected.\n", STRING(entity->v.netname)).c_str());
        }
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };

    func_table.pfnClientDisconnect = [](edict_t* entity) -> void {
        SERVER_PRINT(std::format("POKEBOT: {} is disconnected.\n", STRING(entity->v.netname)).c_str());
        pokebot::plugin::Pokebot::OnClientDisconnect(entity);
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnClientPutInServer = [](edict_t* entity) -> void {
		pokebot::plugin::Pokebot::OnClientPutInServer(entity);
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnServerActivate = [](edict_t* edict_list, int edict_count, int client_max) -> void {
        pokebot::plugin::Pokebot::OnServerActivate(edict_list, edict_count, client_max);

        pokebot::game::game.Init(edict_list, edict_count);
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnClientCommand = [](edict_t* client) -> void {
        pokebot::plugin::Pokebot::OnPlayerMenuSelect(client);
        if (client != pokebot::game::game.host.AsEdict()) {
            return;
        }

        const std::string_view command = CMD_ARGV(0);
        if (command != "menuselect") {
            return;
        }

        const auto arg = CMD_ARGV(0);
        char *not_converted;
        int key = strtol(CMD_ARGV(1), &not_converted, 10);
        pokebot::game::Menu::Instance().OnPress(client, key);
        RETURN_META(MRES_IGNORED);
    };

    memcpy(pFunctionTable, &func_table, sizeof(DLL_FUNCTIONS));
    return (TRUE);
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion) {
    gFunctionTable_Post.pfnSpawn = [](edict_t* entity) -> int {
        if (std::string_view(STRING(entity->v.classname)) == "trigger_multiple") {
            entity->v.flags |= FL_WORLDBRUSH;   // make it detectable!
            entity->v.rendermode = kRenderNormal;
        } else if (std::string_view(STRING(entity->v.classname)) == "worldspawn") {
            worldspawn = entity;
        }
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };
    memcpy(pFunctionTable, &gFunctionTable_Post, sizeof(DLL_FUNCTIONS));
    return (TRUE);
}
