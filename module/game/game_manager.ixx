export module pokebot.game: game_manager;
import pokebot.game.cs_game_manager;

import pokebot.database;
import pokebot.engine;
import pokebot.game.scenario.manager;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.game.entity;
import pokebot.util;
import pokebot.plugin.event;
import pokebot.plugin.event.server_activation;
import pokebot.plugin.observables;

import pokebot.common.event_handler;

export namespace pokebot::game {
	constexpr float Default_Max_Move_Speed = 255.0f;
	class Game;

	class Host {
		edict_t* host{};
	public:
		void ShowMenu();

		const edict_t* AsEdict() const POKEBOT_NOEXCEPT { return host; }
		bool IsHostValid() const POKEBOT_NOEXCEPT;
		void SetHost(edict_t* const target) POKEBOT_NOEXCEPT;
		const char* const HostName() const POKEBOT_NOEXCEPT;
		const Vector& Origin() const POKEBOT_NOEXCEPT;
		void Update();
	};

	class Game : public CSGameBase {
		database::Database database{};

		std::vector<util::fixed_string<32u>> bot_args{};
		MapFlags map_flags{};
		uint32_t round{};
		bool is_newround{};

		std::optional<Vector> c4_origin{};
		edict_t* backpack{};

		std::shared_ptr<scenario::ScenarioManager> scenario_managers[4];
	public:
		Game(plugin::Observables* plugin_observables,
			 engine::Observables* engine_observables);
		
		std::shared_ptr<scenario::ScenarioManager> GetDemolitionManager() const override;

		Host host{};


		size_t GetLives(const Team) const POKEBOT_NOEXCEPT {
			return 0;
		}

		uint32_t CurrentRonud() const POKEBOT_NOEXCEPT {
			return round;
		}

		bool IsCurrentMode(const MapFlags Game_Mode) const POKEBOT_NOEXCEPT {
			return bool(map_flags & Game_Mode);
		}

		MapFlags GetScenario() const POKEBOT_NOEXCEPT {
			return map_flags;
		}
	};
}