#pragma once
#include "bot/troops.hpp"

namespace pokebot::bot {
	enum class Policy {
		Elimination,
		Survival,		// Survive until the round ends.
		Defense,
		Offense,
		Player
	};

	struct RadioMessage {
		common::Team team = common::Team::Spector;
		common::PlayerName sender{};
		common::fixed_string<255u> message{};
		PlatoonID platoon{};
	};

	struct BotBalancer final {
		Vector gap{};
	};

	using BotPair = std::pair<common::PlayerName, Bot>;

	/**
	* @brief Manager class for Bot.
	*/
	class Manager final : private common::Singleton<Manager> {
		std::optional<Vector> c4_origin{};
		common::Array<Troops, 2> troops;
		friend class pokebot::message::MessageDispatcher;
		std::unordered_map<common::PlayerName, Bot, common::PlayerName::Hash> bots{};

		// The name of the player who has the bomb.
		common::PlayerName bomber_name{};

		Timer round_started_timer{};
		enum class InitializationStage { Preparation, Player_Action_Ready, Completed } initialization_stage{};

		Bot* const Get(const std::string_view&) POKEBOT_NOEXCEPT;
		RadioMessage radio_message{};
		Manager();
	public:
		const decltype(bomber_name)& Bomber_Name = bomber_name;
		/**
		* @brief Get the instance of Manager
		* @return The instance of Manager
		*/
		static Manager& Instance() POKEBOT_NOEXCEPT {
			static Manager manager{};
			return manager;
		}

		/**
		* @brief Called when a new round starts.
		*/
		void OnNewRoundPreparation() POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a new round starts.
		*/
		void OnNewRoundReady() POKEBOT_NOEXCEPT;


		/**
		* @brief Update the state of all bots.
		*/
		void Update() POKEBOT_NOEXCEPT;

		/**
		* @brief Insert a new bot into the game.
		* @param bot_name The name of the bot.
		* @param team The team of the bot.
		* @param model The model of the bot.
		* @param difficulty The difficulty level of the bot.
		*/
		void Insert(common::PlayerName bot_name, const common::Team team, const common::Model model, const bot::Difficult difficulty) POKEBOT_NOEXCEPT;

		/**
		* @brief Kick a bot from the game.
		* @param Bot_Name The name of the bot.
		*/
		void Kick(const std::string_view&  Bot_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Remove a bot from the game.
		* @param Bot_Name The name of the bot.
		*/
		void Remove(const std::string_view&  Bot_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Check if a bot exists by name.
		* @param Bot_Name The name of the bot.
		* @return true if the bot exists, false otherwise.
		*/
		bool IsExist(const std::string_view&  Bot_Name) const POKEBOT_NOEXCEPT;

		/**
		* @brief Assign an engine message to a bot.
		* @param Bot_Name The name of the bot.
		* @param message The message to assign.
		*/
		void Assign(const std::string_view& Bot_Name, Message message) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a bot dies.
		* @param Bot_Name The name of the bot.
		*/
		void OnDied(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a bot takes damage.
		* @param Bot_Name The name of the bot.
		* @param Inflictor The entity that inflicted the damage.
		* @param Damage The amount of damage taken.
		* @param Armor The amount of armor remaining.
		* @param Bit Additional information about the damage.
		*/
		void OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a bot joins a team.
		* @param Bot_Name The name of the bot.
		*/
		void OnJoinedTeam(const std::string_view&) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a chat message is received.
		* @param Bot_Name The name of the bot.
		*/
		void OnChatRecieved(const std::string_view&) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a team chat message is received.
		* @param Bot_Name The name of the bot.
		*/
		void OnTeamChatRecieved(const std::string_view&) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a radio message is received.
		* @param Sender_Name The name of the sender.
		* @param Radio_Sentence The radio message.
		*/
		void OnRadioRecieved(const std::string_view& Sender_Name, const std::string_view& Radio_Sentence) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when the bomb is planted.
		*/
		void OnBombPlanted() POKEBOT_NOEXCEPT;

		/**
		* @brief Called when th bomb is picked up by a player.
		* @param Client_Name The name of the player who picked up the bomb.
		*/
		void OnBombPickedUp(const std::string_view& Client_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when th bomb is dropped by a player.
		* @param Client_Name The name of the player who picked up the bomb.
		*/
		void OnBombDropped(const std::string_view& Client_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Called when a bot has completely joined the game.
		* @param bot The bot that joined.
		*/
		void OnBotJoinedCompletely(Bot* const) POKEBOT_NOEXCEPT;

		/**
		* @brief Get the goal node ID for a troop or platoon.
		* @param Target_Team The team of the troop.
		* @param Index The platoon index.
		* @return The goal node ID. If Index is less than 0, returns the troop's goal node ID.
		*/
		node::NodeID GetGoalNode(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT;

		game::Client* GetLeader(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT;
		bool IsFollowerPlatoon(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT;

		void AssertStrategy(const common::Team Target_Team, const PlatoonID Index, TroopsStrategy::Strategy strategy) {
			auto& troop = troops[static_cast<int>(Target_Team) - 1];
			if (Index < 0) {
				troop.AssertStrategy(strategy);
			} else {
				troop.at(*Index).AssertStrategy(strategy);
			}
		}

		/**
		* @brief Get the origin of the C4 bomb.
		* @return The origin of the C4 bomb.
		*/
		const std::optional<Vector>& C4Origin() const POKEBOT_NOEXCEPT { return c4_origin; }
	};
}