export module pokebot: plugin;
import pokebot.api.command_executors;


import pokebot.bot;
import pokebot.common.event_handler;
import pokebot.plugin.event;
import pokebot.plugin.event.server_activation;
import pokebot.plugin.observables;

import pokebot.plugin.console.variable;
import pokebot.terrain.graph.graph_base;
import pokebot.game;
import pokebot.game.client;
import pokebot.game.util;
import pokebot.util;

export namespace pokebot::plugin {
	class Pokebot : public pokebot::api::BotCommandExecutor {
		static edict_t* pWorldEntity;
		static edict_t* spawned_entity;

		static std::vector<console::ConVarReg> convars;
		static Observables observables;

		static std::unique_ptr<pokebot::bot::Manager> bot_manager;
		static std::unique_ptr<pokebot::game::Game> game;
		static std::unique_ptr<pokebot::node::Graph> czworld;
		static std::unique_ptr<pokebot::game::client::ClientManager> clients;
		
		static bool IsPlayable_() noexcept;
		static void AddBot_(const std::string_view& Bot_Name, const game::Team, const game::Model) noexcept;
	public:
        void AddBot(const std::string_view& botName, pokebot::game::Team team, pokebot::game::Model model) override {
            Pokebot::AddBot_(botName, team, model);
        }

		bool IsPlayable() override {
            return Pokebot::IsPlayable_();
        }

		static void AddConsoleVariable(const char* name, const char* value, const char* info, bool bounded, float min, float max, console::Var varType, bool missingAction, const char* regval, console::ConVar* self);
		static void RegisterConsoleVariables() noexcept;


		static void OnDllAttached() noexcept;

		static void AppendSpawnedEntity(edict_t* entity) noexcept;

		static void RegisterCommand() noexcept;
		static void OnUpdate() noexcept;
		
		static void OnServerActivate(edict_t edict_list[], int edict_count, int client_max) noexcept;

		static void OnEntitySpawned() noexcept;
		static void OnClientConnect(edict_t* entity, const char* Address) noexcept;
		static void OnClientDisconnect(const edict_t* const disconnected_client) noexcept;
		static void OnMapLoaded() noexcept;
		static void OnGameInit() noexcept;

		static void OnClientPutInServer(edict_t* client) noexcept;
		static void OnEntitySpawned(edict_t* entity) noexcept;
		static void OnPlayerMenuSelect(edict_t* client) noexcept;
	};
}