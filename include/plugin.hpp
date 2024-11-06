#pragma once
#include "behavior.hpp"

#define POKEBOT_PLUGIN_GOLDSRC 0
#define POKEBOT_PLUGIN_SOURCE 1
#define POKEBOT_PLUGIN POKEBOT_PLUGIN_GOLDSRC

namespace pokebot::plugin {
	inline class Pokebot {
		static inline bool draw_node{};

		int beam_sprite{};
		edict_t* pWorldEntity{};
		edict_t* disconnected_client{}, *spawned_entity{};
	public:
		int BeamSprite() const noexcept { return beam_sprite; }
		void AppendDisconnectedClient(edict_t* client) noexcept { disconnected_client = client; }
		void AppendSpawnedEntity(edict_t* entity) noexcept { spawned_entity = entity; }

		void RegisterCommand() noexcept;
		void OnUpdate() noexcept;
		void AddBot(const std::string& Bot_Name, const common::Team, const common::Model, const bot::Difficult) POKEBOT_DEBUG_NOEXCEPT;
		
		void OnServerInit();
		
		void OnEntitySpawned();
		void OnClientConnect();
		void OnClientDisconnect();
	} pokebot_plugin{};
}