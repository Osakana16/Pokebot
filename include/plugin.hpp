#pragma once

#define POKEBOT_PLUGIN_GOLDSRC 0
#define POKEBOT_PLUGIN_SOURCE 1
#define POKEBOT_PLUGIN POKEBOT_PLUGIN_GOLDSRC

namespace pokebot::plugin {
	inline class Pokebot {
		static inline bool draw_node{};

		int beam_sprite{};
		edict_t* pWorldEntity{};
		edict_t *spawned_entity{};
	public:
		int BeamSprite() const POKEBOT_NOEXCEPT { return beam_sprite; }
		void AppendSpawnedEntity(edict_t* entity) POKEBOT_NOEXCEPT { spawned_entity = entity; }

		void RegisterCommand() POKEBOT_NOEXCEPT;
		void OnUpdate() POKEBOT_NOEXCEPT;
		void AddBot(const std::string_view& Bot_Name, const game::Team, const game::Model) POKEBOT_NOEXCEPT;
		
		void OnServerInit();
		
		void OnEntitySpawned();
		void OnClientConnect();
		void OnClientDisconnect(const edict_t* const disconnected_client);
		void OnMapLoaded();
	} pokebot_plugin{};
}