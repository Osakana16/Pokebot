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

	void Troops::DecideStrategy(std::unordered_map<common::PlayerName, Bot, common::PlayerName::Hash>* const bots , const std::optional<RadioMessage>& radio_message) {
		if (bots->empty())
			return;

#if 1
		TroopsStrategy new_strategy{};
		auto selectGoal = [&](node::GoalKind kind)->node::NodeID {
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
				return it->second;
			}
		};

		auto candidates = (*bots | std::views::filter(commander_condition));
		if (candidates.empty()) {
			if (radio_message.has_value()) {
				new_strategy.strategy = TroopsStrategy::Strategy::Follow;
				new_strategy.leader_name = radio_message->sender.data();
			}
		} else {
			switch (team) {
				case common::Team::T:
				{
					if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
						auto kind = node::GoalKind::Bombspot;
						new_strategy.strategy = TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite_Concentrative;
						switch (new_strategy.strategy) {
							case TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite_Concentrative:
							{
								if (IsRoot()) {
									// If the troop is the root, create new platoons.
									while (DeletePlatoon(0));	// Delete all platoon.

									int platoon = CreatePlatoon(
										[](const BotPair& target) -> bool { return target.second.JoinedPlatoon() == 0; },
										[](const BotPair& target) -> bool { return target.second.HasWeapon(game::Weapon::C4); }
									);

									auto followers = (*bots | std::views::filter([](const BotPair& target) -> bool {
										return target.second.JoinedTeam() == common::Team::T && !target.second.HasWeapon(game::Weapon::C4); }) | std::views::take(5)
											);
									for (auto& follower : followers) {
										follower.second.JoinPlatoon(platoon);
									}
									new_strategy.objective_goal_node = selectGoal(kind);
								} else {
									// If the troop is a platoon
									new_strategy.strategy = TroopsStrategy::Strategy::Follow;
									new_strategy.leader_name = Manager::Instance().Bomber_Name.data();
									assert(!new_strategy.leader_name.empty());
								}
								break;
							}
							default:
								assert(false);
								break;
						}
					} else if (game::game.IsCurrentMode(game::MapFlags::HostageRescue)) {
						auto kind = node::GoalKind::Rescue_Zone;
						strategy.strategy = TroopsStrategy::Strategy::Prevent_Hostages;
						new_strategy.objective_goal_node = selectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Assassination)) {
						auto kind = node::GoalKind::Vip_Safety;
						new_strategy.objective_goal_node = selectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Escape)) {
						auto kind = node::GoalKind::Escape_Zone;
						new_strategy.objective_goal_node = selectGoal(kind);
					}
					break;
				}
				case common::Team::CT:
				{
					if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
						auto kind = node::GoalKind::Bombspot;
						new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
						switch (new_strategy.strategy) {
							case TroopsStrategy::Strategy::Defend_Bombsite_Divided:
							{
								if (IsRoot()) {
									// If the troop is the root, create new platoons.
									while (DeletePlatoon(0));	// Delete all platoon.

									const size_t Number_Of_Goals = node::czworld.GetNumberOfGoals(node::GoalKind::Bombspot);
									assert(Number_Of_Goals > 0);
									for (int i = 0; i < Number_Of_Goals; i++) {
										CreatePlatoon(
											[i](const BotPair& target) -> bool {
											return i == target.second.JoinedPlatoon();
										},
											[i](const BotPair& target) -> bool {
											return i == target.second.JoinedPlatoon();
										}
										);
									}

									auto cts = (*bots | std::views::filter([](const BotPair& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; }));
									const size_t Number_Of_Cts = std::distance(cts.begin(), cts.end());
									if (Number_Of_Cts > 1) {
										const size_t Number_Of_Member_In_Squad = static_cast<size_t>(std::ceil(static_cast<common::Dec>(Number_Of_Cts) / static_cast<common::Dec>(Number_Of_Goals)));
										auto member = cts.begin();
										for (int squad = 0; squad < Number_Of_Goals; squad++) {
											for (int j = 0; j < Number_Of_Member_In_Squad && member != cts.end(); j++, member++) {
												member->second.JoinPlatoon(squad);
											}
										}
									} else {
										new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
										new_strategy.objective_goal_node = selectGoal(kind);
									}
								} else {
									// If the troop is a platoon
									new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
									new_strategy.objective_goal_node = selectGoal(kind);
								}
								break;
							}
							default:
								assert(false);
								break;
						}
					} else if (game::game.IsCurrentMode(game::MapFlags::HostageRescue)) {
						auto kind = node::GoalKind::Rescue_Zone;
						strategy.strategy = TroopsStrategy::Strategy::Rush_And_Rescue;
						new_strategy.objective_goal_node = selectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Assassination)) {
						auto kind = node::GoalKind::Vip_Safety;
						new_strategy.objective_goal_node = selectGoal(kind);
					} else if (game::game.IsCurrentMode(game::MapFlags::Escape)) {
						auto kind = node::GoalKind::Escape_Zone;
						new_strategy.objective_goal_node = selectGoal(kind);
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
}