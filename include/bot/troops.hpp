#pragma once
namespace pokebot::bot {
	struct TroopsStrategy {
		std::string leader_name{};	// The name of the player to follow.
		node::NodeID objective_goal_node = node::Invalid_NodeID;
		enum class Strategy {
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
				Stick together and defend the specific bombsite.
				CTs never split the team. All members defend one bombsite.
			*/
			Defend_Bombsite_Concentrative,
			/*
				Divide the team and defend the bombsite.
				CTs splits into teams based on number of bombing sites and defend bombsites.
			*/
			Defend_Bombsite_Divided,
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

	class Troops final {
		TroopsStrategy strategy;
		TroopsStrategy old_strategy;
		std::function<bool(const std::pair<std::string, Bot>& target)> commander_condition;	// The condition to become the commander.
		std::function<bool(const std::pair<std::string, Bot>& target)> condition;

		common::Team team{};
		Troops* parent{};
		std::vector<Troops> platoons{};
	public:
		common::Team Team() { return team; }
		Troops(decltype(condition) target_condition, decltype(commander_condition) target_commander_condition, decltype(team) target_team) : condition(target_condition), commander_condition(target_commander_condition), team(target_team) {}
		bool IsRoot() const POKEBOT_NOEXCEPT { return parent == nullptr; }
		int CreatePlatoon(decltype(condition) target_condition, decltype(condition) target_commander_condition);
		bool DeletePlatoon(const int Index);
		bool IsStrategyToFollow() const noexcept { return strategy.strategy == TroopsStrategy::Strategy::Follow; }

		void DecideStrategy(std::unordered_map<std::string, Bot>* bots, const std::optional<RadioMessage>&);
		void Command(std::unordered_map<std::string, Bot>* bots);
		void SetNewStrategy(const TroopsStrategy&);
		bool HasGoalBeenDevised(const node::NodeID) const POKEBOT_NOEXCEPT;
		bool HasGoalBeenDevisedByOtherPlatoon(const node::NodeID) const POKEBOT_NOEXCEPT;
		bool NeedToDevise() const POKEBOT_NOEXCEPT;

		node::NodeID GetGoalNode() const POKEBOT_NOEXCEPT { return strategy.objective_goal_node; }
		game::ClientStatus LeaderStatus() const POKEBOT_NOEXCEPT;

		Troops& operator[](const int index) { return platoons[index]; }
		const Troops& at(const int index) const { return platoons.at(index); }

		auto begin() { return platoons.begin(); }
		auto end() { return platoons.end(); }
		auto GetPlatoonSize() { return platoons.size(); }
		void AssertStrategy(TroopsStrategy::Strategy target) const { assert(strategy.strategy == target); }
	};
}