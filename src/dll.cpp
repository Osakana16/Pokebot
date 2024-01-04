#include "plugin.hpp"
#include "graph.hpp"
#include <vector>

enginefuncs_t g_engfuncs;
globalvars_t* gpGlobals;

// START of Metamod stuff

enginefuncs_t meta_engfuncs, meta_engfuncs_post;
gamedll_funcs_t* gpGamedllFuncs;
mutil_funcs_t* gpMetaUtilFuncs;
meta_globals_t* gpMetaGlobals;

DLL_FUNCTIONS func_table;
DLL_FUNCTIONS gFunctionTable_Post;
bool draw_node = false;
int Beam_Sprite;

edict_t* pWorldEntity;

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
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
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

    static auto GetArgs = []() {
        static std::vector<std::string> args{};
        for (int i = 1; ; i++) {
            auto arg = CMD_ARGV(i);
            if (arg == nullptr || strlen(arg) <= 0) {
                break;
            }
            args.push_back(arg);
        }
        return std::move(args);
    };

    REG_SVR_COMMAND(
        "pk_add",
        [] {
            auto args = GetArgs();

            std::string name = "FirstBot";
            pokebot::common::Team team = (pokebot::common::Team)(int)pokebot::common::Random<int>(1, 2);
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            if (args.size() >= 1) {
                name = args[0];
            }

            if (args.size() >= 2) {
                if (args[1] == "1" || args[1] == "T" || args[1] == "t") {
                    team = pokebot::common::Team::T;
                } else if (args[1] == "2" || args[1] == "CT" || args[1] == "ct") {
                    team = pokebot::common::Team::CT;
                }
            }

            if (args.size() >= 3) {
                model = static_cast<decltype(model)>(std::strtol(args[2].c_str(), nullptr, 0) % 4);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name, team, model);
        }
    );

    REG_SVR_COMMAND(
        "pk_add_ct",
        [] {
            auto args = GetArgs();

            std::string name = "FirstBot";
            pokebot::common::Team team = pokebot::common::Team::CT;
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            if (args.size() >= 1) {
                name = args[0];
            }

            if (args.size() >= 2) {
                model = static_cast<decltype(model)>(std::strtol(args[1].c_str(), nullptr, 0) % 4);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name, team, model);
        }
    );

    REG_SVR_COMMAND(
        "pk_add_t",
        [] {
            auto args = GetArgs();

            std::string name = "FirstBot";
            pokebot::common::Team team = pokebot::common::Team::T;
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            if (args.size() >= 1) {
                name = args[0];
            }

            if (args.size() >= 2) {
                model = static_cast<decltype(model)>(std::strtol(args[1].c_str(), nullptr, 0) % 4);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name, team, model);
        }
    );

    REG_SVR_COMMAND(
        "pk_auto_waypoint",
        [] {
            pokebot::game::is_enabled_auto_waypoint = !pokebot::game::is_enabled_auto_waypoint;        
        }
    );

    REG_SVR_COMMAND(
        "pk_draw_waypoint",
        [] {
            draw_node = !draw_node;
        }
    );

    REG_SVR_COMMAND(
        "pk_kill",
        [] {
            auto args = GetArgs();  
            using namespace pokebot;
            MDLL_ClientKill(*game::game.clients.Get(args[0]));
        }
    );

    REG_SVR_COMMAND(
        "pk_kill_t",
        [] {
                    
        }
    );

    REG_SVR_COMMAND(
        "pk_kill_ct",
        [] {
        }
    );

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

C_DLLEXPORT void WINAPI
GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals) {
    // get the engine functions from the engine...
    memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
    gpGlobals = pGlobals;
}

bool is_game_completely_initialized = false;

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion) {
    func_table.pfnStartFrame = []() -> void {
        using namespace pokebot;
        if (is_game_completely_initialized) {
        //    pokebot::node::world.OnNewRound();
            is_game_completely_initialized = false;
        }

        if (draw_node)
            pokebot::node::world.Draw();

        if (pokebot::game::game.GetHost() != nullptr) {
            const_cast<edict_t*>(pokebot::game::game.GetHost())->v.health = 255;
        }
#if 0
        {
            auto host = pokebot::game::game.GetHost();
            if (host != nullptr) {
                TraceResult tr{};
                TRACE_HULL(host->v.origin, host->v.origin + Vector(50, 0, 0), IGNORE_MONSTERS::ignore_monsters, HULL_TYPE::point_hull, nullptr, &tr);
                if (tr.flFraction < 1.0 && tr.pHit != nullptr) {
                    SERVER_PRINT(std::format("{}\n", STRING(tr.pHit->v.classname)).c_str());
                }
            }
        }
#endif

        pokebot::plugin::pokebot_plugin.OnUpdate();
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnGameInit = []() -> void {
        pokebot::node::world.OnMapLoaded();
        // pokebot::ParseWeaponJson();
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnSpawn = [](edict_t* entity) -> int {
        std::string classname = STRING(entity->v.classname);
        if (entity->v.rendermode == kRenderTransTexture) {
            entity->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents
        }
        if (classname == "worldspawn") {
            pWorldEntity = entity;
            Beam_Sprite = PRECACHE_MODEL("sprites/laserbeam.spr");
        }
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };

    func_table.pfnClientConnect = [](edict_t* entity, const char* Name, const char* Address, char Reject_Reason[]) -> qboolean {
        if (gpGlobals->deathmatch) {
            // check if this client is the listen server client
            if (strcmp(Address, "loopback") == 0) {
                // save the edict of the listen server client...
                pokebot::game::game.SetHost(entity);
                is_game_completely_initialized = true;
            }
        }
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };

    func_table.pfnClientDisconnect = [](edict_t* entity) -> void {
        pokebot::bot::manager.Remove(STRING(entity->v.netname));
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnClientPutInServer = [](edict_t* entity) -> void {
        RETURN_META(MRES_IGNORED);
    };

    func_table.pfnServerActivate = [](edict_t* edictList, int edictCount, int) -> void {
        pokebot::node::world.Clear();
        pokebot::game::game.Init(edictList, edictCount);
    };

    func_table.pfnClientCommand = [](edict_t*) -> void {
        RETURN_META(MRES_IGNORED);
    };

    memcpy(pFunctionTable, &func_table, sizeof(DLL_FUNCTIONS));
    return (TRUE);
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion) {
    gFunctionTable_Post.pfnSpawn = [](edict_t* entity) -> int {
        if (FStrEq(STRING(entity->v.classname), "trigger_multiple")) {
            entity->v.flags |= FL_WORLDBRUSH;   // make it detectable!
            entity->v.rendermode = kRenderNormal;
        }
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };
    memcpy(pFunctionTable, &gFunctionTable_Post, sizeof(DLL_FUNCTIONS));
    return (TRUE);
}

namespace pokebot::plugin {
    void Pokebot::OnUpdate() noexcept {
        if (game::is_enabled_auto_waypoint && pokebot::game::game.HasHost()) {
            pokebot::node::world.Add(pokebot::game::game.GetHost()->v.origin, pokebot::node::GoalKind::None);
        }
        pokebot::game::game.Update();
        pokebot::bot::manager.Update();
    }

    void Pokebot::AddBot(const std::string& Bot_Name, const common::Team Selected_Team, const common::Model Selected_Model) POKEBOT_DEBUG_NOEXCEPT {
        pokebot::bot::manager.Insert(Bot_Name, Selected_Team, Selected_Model);
    }
}