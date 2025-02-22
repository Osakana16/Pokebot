#pragma once
namespace pokebot::bot {
	struct TroopsStrategy {
		common::PlayerName leader_name{};	// The name of the player to follow.
		node::NodeID objective_goal_node = node::Invalid_NodeID;
		std::optional<int> hostage_id{};

		enum class Strategy {
			None,
			/*- Demolition -*/

			/*
				Stick together and plant the bomb on the specific bombsite.
				Terrorists never split the team. All members defend one bombsite.
			*/
			Plant_C4_Specific_Bombsite_Concentrative,
			/*
				Divide the team into the bomber and the no-bomber.
			*/
			Plant_C4_And_Deceive,
			/*
				The bomb is dropped, take it hurry!
			*/
			Take_Backpack,
			/*
				Stick together and defend the specific bombsite.
				CTs never split the team. All members defend one bombsite.
			*/
			Defend_Bombsite_Concentrative,
			/*
				Divide the team and defend the bombsite.
				CTs splits into teams based on number of bombing sites and defend bombsites.
			*/
			Defend_Bombsite_Divided,
			/*
				I found the backpack, help me defend it!			
			*/
			Defend_Backpack,
			/*- Hostage Rescue -*/
			Prevent_Hostages,
			Prevent_Rescuezones,
			Rush_And_Rescue,
			/*- Assasination -*/
			Prevent_Safety,
			Rush_Safety,
			/*- Escape -*/
			Rush_Escapezone,
			Prevent_Escapezone,
			/*- Common -*/
			Follow
		} strategy;
	};

	using Bots = std::unordered_map<common::PlayerName, Bot, common::PlayerName::Hash>;

	class Troops final {
		TroopsStrategy strategy;
		TroopsStrategy old_strategy;

		std::function<bool(const std::pair<common::PlayerName, Bot>& target)> condition;

		common::Team team{};			// The team of the platoon.

		Troops* parent{};				// My parent troop.

		std::vector<Troops> platoons{};	// The children of troops.

		/**
		* @brief Delete all platoons.
		*/
		inline void DeleteAllPlatoon() noexcept { while (DeletePlatoon(0)); }

		node::NodeID SelectGoal(node::GoalKind kind);


		/**
		* 
		*/
		void DecideSpecialStrategy(Bots* bots, TroopsStrategy* new_strategy, const std::optional<RadioMessage>& radio_message);

		void DecideStrategyToRescueHostageSplit(Bots* bots, TroopsStrategy* new_strategy);
		void DecideStrategyToPlantC4Concentrative(Bots* bots, TroopsStrategy* new_strategy);
		void DecideStrategyToDefendBombsite(Bots* bots, TroopsStrategy* new_strategy);
	public:
		common::Team Team() { return team; }
		Troops(decltype(condition) target_condition, decltype(team) target_team) : condition(target_condition), team(target_team) {}

		/**
		* @brief Check the troop is the root.
		* @return True if the troop is the root, false if not.
		*/
		inline bool IsRoot() const POKEBOT_NOEXCEPT { return parent == nullptr; }

		/**
		* @brief Delete the specified platoon.
		* @param Index The index of the platoon.
		*/
		inline bool DeletePlatoon(const int Index) { return !platoons.empty() && platoons.erase(platoons.begin() + Index) != platoons.end(); }

		/**
		* @brief Create new platoon.
		* @param target_condition 
		* @param target_commander_condition 
		* @return Platoon index.
		*/
		int CreatePlatoon(decltype(condition) target_condition) noexcept;
		
		/**
		* @brief 
		* @return 
		*/
		bool IsStrategyToFollow() const noexcept { return strategy.strategy == TroopsStrategy::Strategy::Follow; }

		/**
		* @brief 
		* @param bots
		* @param radio_message
		*/
		void DecideStrategy(Bots* bots, const std::optional<RadioMessage>& radio_message);

		/**
		* @brief 
		* @param 
		*/
		void Command(Bots* bots);

		/**
		* @brief Set the new strategy and apply it to own platoons.
		* @param TroopStrategy The new strategy.
		*/
		void SetNewStrategy(const TroopsStrategy&) noexcept;

		/**
		* @brief 
		* @param 
		*/
		bool HasGoalBeenDevised(const node::NodeID) const POKEBOT_NOEXCEPT;
		
		/**
		* @brief 
		* @param 
		*/
		bool HasGoalBeenDevisedByOtherPlatoon(const node::NodeID) const POKEBOT_NOEXCEPT;
		
		/**
		* @brief Check the troop has not decided the strategy yet.
		* @param 
		*/
		bool NeedToDevise() const POKEBOT_NOEXCEPT;

		/**
		* @brief Get tartet hostage index.
		* @return Target hostage index.
		*/
		std::optional<int> GetTargetHostageIndex() const noexcept { return strategy.hostage_id; }
		
		/**
		* @brief 
		* @return 
		*/
		node::NodeID GetGoalNode() const noexcept { return strategy.objective_goal_node; }

		/**
		* @brief 
		* @return 
		*/
		game::Client* GetLeader() const POKEBOT_NOEXCEPT;


		Troops& operator[](const int index) { return platoons[index]; }
		const Troops& at(const int index) const { return platoons.at(index); }

		auto begin() { return platoons.begin(); }
		auto end() { return platoons.end(); }
		auto GetPlatoonSize() { return platoons.size(); }
		void AssertStrategy(TroopsStrategy::Strategy target) const { assert(strategy.strategy == target); }
	};
}