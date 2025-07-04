export module pokebot: plugin;

import pokebot.bot;
import pokebot.common.event_handler;
import pokebot.plugin.event;
import pokebot.plugin.event.map_loaded;
import pokebot.plugin.event.client_connection;
import pokebot.plugin.event.client_disconnection;

import pokebot.game;
import pokebot.game.util;

export namespace pokebot::plugin {
	class Pokebot {
		static edict_t* pWorldEntity;
		static edict_t* spawned_entity;

		inline static plugin::event::ClientConnectionObservable client_connection_observable{};
		inline static plugin::event::ClientDisconnectionObservable client_disconnection_observable{};
		inline static plugin::event::MapLoadedObservable map_loaded_observable{};

		inline static std::unique_ptr<pokebot::bot::Manager> bot_manager{};
		inline static std::unique_ptr<pokebot::game::Game> game{};
	public:
		static void AppendSpawnedEntity(edict_t* entity) noexcept;

		static void RegisterCommand() noexcept;
		static void OnUpdate() noexcept;
		static void AddBot(const std::string_view& Bot_Name, const game::Team, const game::Model) noexcept;
		
		static void OnServerInit();
		
		static void OnEntitySpawned();
		static void OnClientConnect();
		static void OnClientDisconnect(const edict_t* const disconnected_client);
		static void OnMapLoaded();

		static void OnClientPutIn(edict_t* client);
		static void OnPlayerMenuSelect(edict_t* client);
	};
}