module;
#include "navmesh/navigation_map.h"

module pokebot: plugin;
import pokebot.bot.behavior;
import std;
import pokebot.bot.manager;
import pokebot.game;
import pokebot.game.client.manager;
import pokebot.util.random;
import pokebot.game.util;
import pokebot.terrain.graph.cznav_graph;
import pokebot.plugin.console.command;
import pokebot.plugin.console.variable;

import pokebot.engine;

namespace pokebot {
    edict_t* plugin::Pokebot::pWorldEntity;
    edict_t* plugin::Pokebot::spawned_entity;
    std::vector<plugin::console::ConVarReg> plugin::Pokebot::convars;

    std::unique_ptr<common::NameManger<bot::Bot>> plugin::Pokebot::bot_manager;
    std::unique_ptr<game::GameBase> plugin::Pokebot::game;
    std::unique_ptr<node::Graph> plugin::Pokebot::czworld;
    std::unique_ptr<common::NameManger<game::client::Client>> plugin::Pokebot::clients;

    plugin::Observables plugin::Pokebot::observables;
}

namespace pokebot::plugin {
    void Pokebot::OnDllAttached() noexcept {
        api::command_executor = std::make_unique<Pokebot>();

        auto callback = [](const event::EdictList& event) {
            Pokebot::game = std::make_unique<pokebot::game::Game>(
                &Pokebot::observables,
                &engine::EngineInterface::observables
            );

            Pokebot::czworld = std::make_unique<pokebot::node::CZBotGraph>(
                &Pokebot::observables,
                &engine::EngineInterface::observables
            );

            Pokebot::clients = std::make_unique<pokebot::game::client::ClientManager>(
                &Pokebot::observables,
                &engine::EngineInterface::observables
            );

            Pokebot::bot_manager = std::make_unique<pokebot::bot::Manager>(
                static_cast<pokebot::game::CSGameBase&>(*Pokebot::game),
                *Pokebot::czworld,
                *Pokebot::clients,
                &Pokebot::observables,
                &engine::EngineInterface::observables
            );

            Pokebot::RegisterConsoleVariables();


            for (int i = 0; i < event.client_max; ++i) {
                auto ent = event.edict_list + i;
                if (const std::string_view classname = STRING(ent->v.classname); classname == "info_player_start" || classname == "info_vip_start") {
                    ent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
                    ent->v.renderamt = 127; // set its transparency amount
                    ent->v.effects |= EF_NODRAW;
                } else if (classname == "info_player_deathmatch") {
                    ent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
                    ent->v.renderamt = 127; // set its transparency amount
                    ent->v.effects |= EF_NODRAW;
                }
            }
        };
        
        pokebot::bot::behavior::DefineBehavior();

        observables.server_activation_observable.AddObserver(std::make_shared<common::NormalObserver<event::EdictList>>(callback));
    }

    void Pokebot::RegisterCommand() noexcept {
        console::CommandRegister{};
    }

    void Pokebot::OnUpdate() noexcept {
        observables.frame_update_observable.NotifyObservers();
    }

    void Pokebot::AddBot_(const std::string_view& Bot_Name, const game::Team Selected_Team, const game::Model Selected_Model) noexcept {
        // bot_manager->Insert(Bot_Name.data(), Selected_Team, *clients, Selected_Model);
    }

    void Pokebot::OnEntitySpawned() noexcept {
        if (spawned_entity->v.rendermode == kRenderTransTexture) {
            spawned_entity->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents
        }

        if (std::string_view classname = STRING(spawned_entity->v.classname); classname == "worldspawn") {
            pWorldEntity = spawned_entity;
        }
    }

    void Pokebot::OnClientConnect(edict_t* entity, const char* Address) noexcept {
        observables.client_connection_observable.NotifyObservers({ .entity = entity, .Address = Address });
    }

    void Pokebot::OnClientDisconnect(const edict_t* const disconnected_client) noexcept {
        observables.client_disconnection_observable.NotifyObservers({ .entity = disconnected_client, .Address = nullptr });
    }

    void Pokebot::OnServerActivate(edict_t edict_list[], int edict_count, int client_max) noexcept {
        observables.server_activation_observable.NotifyObservers({ .edict_list = edict_list, .edict_count = edict_count, .client_max = client_max });
    }


    void Pokebot::OnMapLoaded() noexcept {
        observables.map_loaded_observable.NotifyObservers(STRING(gpGlobals->mapname));
    }


    void Pokebot::OnClientPutInServer(edict_t* client) noexcept {
        observables.client_put_in_server_observable.NotifyObservers(client);
    }

    void Pokebot::OnPlayerMenuSelect(edict_t* client) noexcept {

    }

    void Pokebot::OnGameInit() noexcept {
        observables.game_init_observable.NotifyObservers();
    }

    void Pokebot::OnEntitySpawned(edict_t* entity) noexcept {
        observables.entity_spawn_obserable.NotifyObservers(entity);
    }

    void Pokebot::AppendSpawnedEntity(edict_t* entity) noexcept { spawned_entity = entity; }

    bool Pokebot::IsPlayable_() noexcept {
        return czworld->IsNavFileLoaded();
    }

    void Pokebot::AddConsoleVariable(const char* name, const char* value, const char* info, bool bounded, float min, float max, console::Var varType, bool missingAction, const char* regval, console::ConVar* self) {
        console::ConVarReg reg{
            .reg = {
                .name = name,
                .string = value,
                .flags = FCVAR_EXTDLL
            },
            .info = info,
            .init = value,
            .regval = regval,
            .self = self,
            .initial = (float)std::atof(value),
            .min = min,
            .max = max,
            .missing = missingAction,
            .bounded = bounded,
            .type = varType
        };

        switch (varType) {
            case console::Var::ReadOnly:
                reg.reg.flags |= FCVAR_SPONLY | FCVAR_PRINTABLEONLY;
                [[fallthrough]];
            case console::Var::Normal:
                reg.reg.flags |= FCVAR_SERVER;
                break;
            case console::Var::Password:
                reg.reg.flags |= FCVAR_PROTECTED;
                break;
        }
        convars.push_back(reg);
    }

    void Pokebot::RegisterConsoleVariables() noexcept {
        for (auto& var : convars) {
            console::ConVar& self = *var.self;
            cvar_t& reg = var.reg;
            self.ptr = g_engfuncs.pfnCVarGetPointer(reg.name);

            if (!self.ptr) {
                g_engfuncs.pfnCVarRegister(&var.reg);
                self.ptr = g_engfuncs.pfnCVarGetPointer(reg.name);
            }
        }
    }
}

namespace pokebot::plugin::console {
    ConVar::ConVar(const char* name, const char* initval, Var type, bool regMissing, const char* regVal) {
        plugin::Pokebot::AddConsoleVariable(name, initval, "", false, 0.0f, 0.0f, type, regMissing, regVal, this);
    }

    ConVar::ConVar(const char* name, const char* initval, const char* info, bool bounded, float min, float max, Var type, bool regMissing, const char* regVal) {
       plugin::Pokebot::AddConsoleVariable(name, initval, info, bounded, min, max, type, regMissing, regVal, this);
    }

    // Fix for Static Initialization Order Fiasco
	ConVar poke_freeze{ "pk_freeze", "0" };
	ConVar poke_fight{ "pk_fight", "1" };
	ConVar poke_buy{ "pk_buy", "1" };
}