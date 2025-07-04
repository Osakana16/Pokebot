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
    void Pokebot::RegisterCommand() noexcept {
        console::CommandRegister{};

        REG_SVR_COMMAND("pk_navload", [] {
            pokebot::node::czworld.OnMapLoaded();
        });
    }

    void Pokebot::OnUpdate() noexcept {
        pokebot::game::game.PreUpdate();
        pokebot::bot::Manager::Instance().Update();
        pokebot::game::game.PostUpdate();
    }

    void Pokebot::AddBot(const std::string_view& Bot_Name, const game::Team Selected_Team, const game::Model Selected_Model) noexcept {
        pokebot::bot::Manager::Instance().Insert(Bot_Name.data(), Selected_Team, Selected_Model);
    }

    void Pokebot::OnEntitySpawned() {
        if (spawned_entity->v.rendermode == kRenderTransTexture) {
            spawned_entity->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents
        }

        if (std::string_view classname = STRING(spawned_entity->v.classname); classname == "worldspawn") {
            pWorldEntity = spawned_entity;
        }
    }

    void Pokebot::OnClientConnect() {
        client_connection_observable.Notifyobservers({});
    }

    void Pokebot::OnClientDisconnect(const edict_t* const disconnected_client) {
        client_disconnection_observable.Notifyobservers({ .entity = disconnected_client, .Address = nullptr });
        pokebot::bot::Manager::Instance().Remove(STRING(disconnected_client->v.netname));
        pokebot::game::game.clients.Disconnect(STRING(disconnected_client->v.netname));
    }


    void Pokebot::OnMapLoaded() {
        map_loaded_observable.Notifyobservers(0);
        pokebot::node::czworld.OnMapLoaded();
        pokebot::bot::Manager::Instance().OnMapLoaded();
    }


    void Pokebot::OnClientPutIn(edict_t* client) {


    }

    void Pokebot::OnPlayerMenuSelect(edict_t* client) {

    }


    void Pokebot::AppendSpawnedEntity(edict_t* entity) noexcept { spawned_entity = entity; }
}