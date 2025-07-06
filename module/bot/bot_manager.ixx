module;
export module pokebot.bot: bot_manager;
import :player_ai;
import :radio_message;
import pokebot.common.event_handler;

import pokebot.engine;
import pokebot.terrain.graph;
import pokebot.bot.squad;

export namespace pokebot::bot {
	using BotPair = std::pair<pokebot::util::PlayerName, Bot>;

	/**
	* @brief Manager class for Bot.
	*/
	class Manager final : private game::Singleton<Manager> {
		std::unique_ptr<pokebot::bot::squad::Troops> terrorist_troops;
		std::unique_ptr<pokebot::bot::squad::Troops> ct_troops;

		std::unordered_map<pokebot::util::PlayerName, Bot, pokebot::util::PlayerName::Hash> bots{};

		// The name of the player who has the bomb.
		pokebot::util::PlayerName bomber_name{};

		util::Timer round_started_timer{ util::GetRealGlobalTime };
		enum class InitializationStage { Preparation, Player_Action_Ready, Completed } initialization_stage{};

		Bot* const Get(const std::string_view&) noexcept;
		const Bot* const Get(const std::string_view&) const noexcept;
		RadioMessage radio_message{};

		node::Graph& graph;
	public:
		Manager(node::Graph&,
				common::Observable<void>* frame_update_observable,
				engine::Observables*);

		const decltype(bomber_name)& Bomber_Name = bomber_name;

		/**
		* @brief Called when a new round starts.
		*/
		void OnNewRoundPreparation() noexcept;

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
		void Insert(pokebot::util::PlayerName bot_name, const game::Team team, const game::Model model) POKEBOT_NOEXCEPT;

		/**
		* @brief Kick a bot from the game.
		* @param Bot_Name The name of the bot.
		*/
		void Kick(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Remove a bot from the game.
		* @param Bot_Name The name of the bot.
		*/
		void Remove(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT;

		/**
		* @brief Check if a bot exists by name.
		* @param Bot_Name The name of the bot.
		* @return true if the bot exists, false otherwise.
		*/
		bool IsExist(const std::string_view& Bot_Name) const POKEBOT_NOEXCEPT;

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

		void OnMapLoaded();

		node::NodeID GetGoalNode(const std::string_view& Client_Name) const noexcept;
	};
}