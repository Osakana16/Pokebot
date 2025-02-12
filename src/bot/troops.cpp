#include "bot/manager.hpp"

namespace pokebot::bot {
	bool Troops::HasGoalBeenDevised(const node::NodeID target_objective_node) const POKEBOT_NOEXCEPT {
		return old_strategy.objective_goal_node == target_objective_node;
		// return common::Distance(node::czworld.GetOrigin(old_strategy.objective_goal_node), node::czworld.GetOrigin(target_objective_node)) <= 500.0f;
	}
	bool Troops::HasGoalBeenDevisedByOtherPlatoon(const node::NodeID target_objective_node) const POKEBOT_NOEXCEPT {
		for (auto& platoon : parent->platoons) {
			if (&platoon == this)
				continue;

			if (platoon.strategy.objective_goal_node == target_objective_node) {
				return true;
			}
		}
		return false;
	}

	bool Troops::NeedToDevise() const POKEBOT_NOEXCEPT {
		return strategy.objective_goal_node == node::Invalid_NodeID;
	}

	game::Client* Troops::GetLeader() const POKEBOT_NOEXCEPT {
		assert(!strategy.leader_name.empty());
		auto leader_client = game::game.clients.Get(strategy.leader_name.c_str());
		return const_cast<game::Client*>(leader_client);
	}

	int Troops::CreatePlatoon(decltype(condition) target_condition, decltype(condition) target_commander_condition) {
		platoons.push_back({ target_condition, target_commander_condition, Team() });
		platoons.back().parent = this;
		return platoons.size() - 1;
	}

	bool Troops::DeletePlatoon(const int Index) {
		return !platoons.empty() && platoons.erase(platoons.begin() + Index) != platoons.end();
	}

	void Troops::DecideStrategy(std::unordered_map<common::PlayerName, Bot, common::PlayerName::Hash>* const bots, const std::optional<RadioMessage>& radio_message) {
		if (bots->empty())
			return;

#if 1
		TroopsStrategy new_strategy{};
		auto candidates = (*bots | std::views::filter(commander_condition));
		if (candidates.empty()) {
			if (radio_message.has_value()) {
				if (radio_message->message == "#Take_Bomb") {
					assert(Team() == common::Team::T);
					new_strategy.strategy = TroopsStrategy::Strategy::Take_Backpack;
					new_strategy.leader_name.clear();
				} else {
					new_strategy.strategy = TroopsStrategy::Strategy::Follow;
					new_strategy.leader_name = radio_message->sender.data();
				}
			} else {
				if (!IsRoot()) {
					if (team == common::Team::T) {
						new_strategy.strategy = TroopsStrategy::Strategy::Follow;
						new_strategy.leader_name = Manager::Instance().Bomber_Name.data();
					}
				}
			}
		} else {
			switch (team) {
				case common::Team::T:
				{
					if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
						new_strategy.strategy = TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite_Concentrative;
						switch (new_strategy.strategy) {
							case TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite_Concentrative:
							{
								DecideStrategyToPlantC4Concentrative(bots, &new_strategy);
								break;
							}
							default:
								assert(false);
								break;
						}
					} else if (game::game.IsCurrentMode(game::MapFlags::HostageRescue)) {
						auto kind = node::GoalKind::Rescue_Zone;
						strategy.strategy = TroopsStrategy::Strategy::Prevent_Hostages;
						new_strategy.objective_goal_node = SelectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Assassination)) {
						auto kind = node::GoalKind::Vip_Safety;
						new_strategy.objective_goal_node = SelectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Escape)) {
						auto kind = node::GoalKind::Escape_Zone;
						new_strategy.objective_goal_node = SelectGoal(kind);
					}
					break;
				}
				case common::Team::CT:
				{
					if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
						new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
						switch (new_strategy.strategy) {
							case TroopsStrategy::Strategy::Defend_Bombsite_Divided:
							{
								DecideStrategyToDefendBombsite(bots, &new_strategy);
								break;
							}
							default:
								assert(false);
								break;
						}
					} else if (game::game.IsCurrentMode(game::MapFlags::HostageRescue)) {
						auto kind = node::GoalKind::Rescue_Zone;
						strategy.strategy = TroopsStrategy::Strategy::Rush_And_Rescue;
						new_strategy.objective_goal_node = SelectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Assassination)) {
						auto kind = node::GoalKind::Vip_Safety;
						new_strategy.objective_goal_node = SelectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Escape)) {
						auto kind = node::GoalKind::Escape_Zone;
						new_strategy.objective_goal_node = SelectGoal(kind);
					}
					break;
				}
				default:
					assert(false);
			}
		}

		SetNewStrategy(new_strategy);
#else
		auto candidates = (*bots | std::views::filter(commander_condition));
		if (candidates.empty())
			return;

		switch (Bot* leader = &candidates.front().second; leader->JoinedTeam()) {
			case common::Team::T:
				DecideStrategyForT(bots);
				break;
			case common::Team::CT:
				break;
		}
#endif
	}

	void Troops::Command(std::unordered_map<common::PlayerName, Bot, common::PlayerName::Hash>* bots) {
		for (auto& individual : (*bots | std::views::filter(condition))) {
			individual.second.ReceiveCommand(strategy);
		}
	}

	void Troops::SetNewStrategy(const TroopsStrategy& New_Team_Strategy) {
		old_strategy = strategy;
		strategy = New_Team_Strategy;

		for (auto& platoon : platoons) {
			platoon.SetNewStrategy(strategy);
		}
	}

	void Troops::DeleteAllPlatoon() noexcept {
		while (DeletePlatoon(0));	// Delete all platoon.
	}

	node::NodeID Troops::SelectGoal(node::GoalKind kind) {
#if !USE_NAVMESH
		auto goal = node::world.GetGoal(kind);
#else
		auto goal = node::czworld.GetGoal(kind);
#endif
		for (auto it = goal.first; it != goal.second; it++) {
			if (IsRoot()) {
				if (HasGoalBeenDevised(it->second)) {
					continue;
				}
			} else {
				if (HasGoalBeenDevisedByOtherPlatoon(it->second)) {
					continue;
				}
			}
			// 
			return it->second;
		}
	}

	namespace filter {
		template<common::Team team> bool ByTeam(const BotPair& target) noexcept { return target.second.JoinedTeam() == team; }
		template<game::Weapon weapon> bool ByHavingWeapon(const BotPair& target) noexcept { return target.second.HasWeapon(weapon); }
		template<game::Weapon weapon> bool ByNotHavingWeapon(const BotPair& target) noexcept { return !target.second.HasWeapon(weapon); }
	}

	void Troops::DecideStrategyToPlantC4Concentrative(Bots* bots, TroopsStrategy* new_strategy) {
		auto kind = node::GoalKind::Bombspot;
		if (IsRoot()) {
			// If the troop is the root, create new platoons.
			DeleteAllPlatoon();

			int platoon = CreatePlatoon(
				[](const BotPair& target) -> bool { return target.second.JoinedPlatoon() == 0; },
				filter::ByHavingWeapon<game::Weapon::C4>
			);

			auto terrorists = (*bots | std::views::filter(filter::ByTeam<common::Team::T>));
			auto bomber = (terrorists | std::views::filter(filter::ByHavingWeapon<game::Weapon::C4>));
			auto followers = (terrorists | std::views::filter(filter::ByNotHavingWeapon<game::Weapon::C4>) | std::views::take(5));
			for (auto& follower : followers) {
				follower.second.JoinPlatoon(platoon);
			}
			// If the bot has a C4.
			new_strategy->objective_goal_node = SelectGoal(kind);
		} else {
			// If the troop is a platoon
			new_strategy->strategy = TroopsStrategy::Strategy::Follow;
			new_strategy->leader_name = Manager::Instance().Bomber_Name.data();
			assert(!new_strategy->leader_name.empty());
		}
	}

	void Troops::DecideStrategyToDefendBombsite(Bots* bots, TroopsStrategy* new_strategy) {
		auto kind = node::GoalKind::Bombspot;
		if (IsRoot()) {
			// If the troop is the root, create new platoons.
			DeleteAllPlatoon();

			auto cts = (*bots | std::views::filter(filter::ByTeam<common::Team::CT>));
			const size_t Number_Of_Cts = std::distance(cts.begin(), cts.end());
			if (Number_Of_Cts > 1) {
				const size_t Number_Of_Goals = node::czworld.GetNumberOfGoals(node::GoalKind::Bombspot);
				assert(Number_Of_Goals > 0);
				for (int i = 0; i < Number_Of_Goals; i++) {
					CreatePlatoon(
						[i](const BotPair& target) -> bool { return i == target.second.JoinedPlatoon(); },
						[i](const BotPair& target) -> bool { return i == target.second.JoinedPlatoon(); }
					);
				}

				const size_t Number_Of_Member_In_Squad = static_cast<size_t>(std::ceil(static_cast<common::Dec>(Number_Of_Cts) / static_cast<common::Dec>(Number_Of_Goals)));
				auto member = cts.begin();
				for (int squad = 0; squad < Number_Of_Goals; squad++) {
					for (int j = 0; j < Number_Of_Member_In_Squad && member != cts.end(); j++, member++) {
						member->second.JoinPlatoon(squad);
					}
				}
			} else {
				new_strategy->strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
				new_strategy->objective_goal_node = SelectGoal(kind);
			}
		} else {
			// If the troop is a platoon
			new_strategy->strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
			new_strategy->objective_goal_node = SelectGoal(kind);
		}
	}
}