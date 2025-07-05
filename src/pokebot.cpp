module;
#include "navmesh/navigation_map.h"

module pokebot: plugin;

import std;
import pokebot.bot;
import pokebot.game;
import pokebot.util.random;
import pokebot.game.util;
import pokebot.terrain.graph;
import pokebot.plugin.console.command;

edict_t* pokebot::plugin::Pokebot::pWorldEntity;
edict_t* pokebot::plugin::Pokebot::spawned_entity;

namespace pokebot::plugin {
    void Pokebot::OnDllAttached() noexcept {
        class ServerActivationObserver : public common::Observer<event::EdictList> {
        public:
            ~ServerActivationObserver() final {}

            void OnEvent(const event::EdictList& event) final {
                Pokebot::game = std::make_unique<pokebot::game::Game>(&Pokebot::frame_update_observable, &Pokebot::client_connection_observable, &Pokebot::client_disconnection_observable);
                Pokebot::bot_manager = std::make_unique<pokebot::bot::Manager>(&Pokebot::frame_update_observable);
            }
        };

        server_activation_observable.AddObserver(std::make_shared<ServerActivationObserver>());
    }

    void Pokebot::RegisterCommand() noexcept {
        console::CommandRegister{};

        REG_SVR_COMMAND("pk_navload", [] {
            pokebot::node::czworld.OnMapLoaded();
        });
    }

    void Pokebot::OnUpdate() noexcept {
		frame_update_observable.Notifyobservers();

        pokebot::game::game.PreUpdate();
        pokebot::bot::Manager::Instance().Update();
        pokebot::game::game.PostUpdate();
    }

    void Pokebot::AddBot(const std::string_view& Bot_Name, const game::Team Selected_Team, const game::Model Selected_Model) noexcept {
        pokebot::bot::Manager::Instance().Insert(Bot_Name.data(), Selected_Team, Selected_Model);
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
        client_connection_observable.Notifyobservers({ .entity=entity, .Address=Address });
    }

    void Pokebot::OnClientDisconnect(const edict_t* const disconnected_client) noexcept {
        client_disconnection_observable.Notifyobservers({ .entity = disconnected_client, .Address = nullptr });
        pokebot::bot::Manager::Instance().Remove(STRING(disconnected_client->v.netname));
        pokebot::game::game.clients.Disconnect(STRING(disconnected_client->v.netname));
    }
    
    void Pokebot::OnServerActivate(edict_t edict_list[], int edict_count, int client_max) noexcept {
        server_activation_observable.Notifyobservers({ .edict_list = edict_list, .edict_count = edict_count, .client_max = client_max });
    }


    void Pokebot::OnMapLoaded() noexcept {
        map_loaded_observable.Notifyobservers(STRING(gpGlobals->mapname));
        pokebot::node::czworld.OnMapLoaded();
        pokebot::bot::Manager::Instance().OnMapLoaded();
    }


    void Pokebot::OnClientPutInServer(edict_t* client) noexcept {
		client_put_in_server_observable.Notifyobservers(client);
    }

    void Pokebot::OnPlayerMenuSelect(edict_t* client) noexcept {

    }

    void Pokebot::OnGameInit() noexcept {
        game_init_observable.Notifyobservers();
	}

    void Pokebot::OnEntitySpawned(edict_t* entity) noexcept {
        entity_spawn_obserable.Notifyobservers(entity);
	}

    void Pokebot::AppendSpawnedEntity(edict_t* entity) noexcept { spawned_entity = entity; }
}