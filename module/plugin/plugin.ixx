export module pokebot: plugin;

import pokebot.bot;
import pokebot.common.event_handler;
import pokebot.plugin.event;
import pokebot.plugin.event.server_activation;

import pokebot.terrain.graph;
import pokebot.util;
import pokebot.game;
import pokebot.game.util;

export namespace pokebot::plugin {
	class Pokebot {
		static edict_t* pWorldEntity;
		static edict_t* spawned_entity;

		inline static common::NormalObservable<plugin::event::ClientInformation> client_connection_observable{};
		inline static common::NormalObservable<plugin::event::ClientInformation> client_disconnection_observable{};
		inline static common::NormalObservable<edict_t*> client_put_in_server_observable{};
		inline static common::NormalObservable<util::fixed_string<256u>> map_loaded_observable{};
		inline static common::NormalObservable<plugin::event::EdictList> server_activation_observable{};
		inline static common::NormalObservable<void> frame_update_observable{};
		inline static common::NormalObservable<void> game_init_observable{};
		inline static common::NormalObservable<edict_t*> entity_spawn_obserable{};

		inline static std::unique_ptr<pokebot::bot::Manager> bot_manager{};
		inline static std::unique_ptr<pokebot::game::Game> game{};
		inline static std::unique_ptr<pokebot::node::Graph> czworld{};
	public:
		static bool IsPlayable() noexcept;

		static void OnDllAttached() noexcept;

		static void AppendSpawnedEntity(edict_t* entity) noexcept;

		static void RegisterCommand() noexcept;
		static void OnUpdate() noexcept;
		static void AddBot(const std::string_view& Bot_Name, const game::Team, const game::Model) noexcept;

		static void OnServerActivate(edict_t edict_list[], int edict_count, int client_max) noexcept;

		static void OnEntitySpawned() noexcept;
		static void OnClientConnect(edict_t* entity, const char* Address) noexcept;
		static void OnClientDisconnect(const edict_t* const disconnected_client) noexcept;
		static void OnMapLoaded() noexcept;
		static void OnGameInit() noexcept;

		static void OnClientPutInServer(edict_t* client) noexcept;
		static void OnEntitySpawned(edict_t* entity) noexcept;
		static void OnPlayerMenuSelect(edict_t* client) noexcept;

		
		static void OnMessageBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* edict);
		static void OnMessageEnd();
	};
}