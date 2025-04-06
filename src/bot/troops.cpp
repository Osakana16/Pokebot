#include "bot/manager.hpp"

namespace pokebot::bot {
	void Troops::Init() {
		strategy = old_strategy = TroopsStrategy();
		platoons.clear();
	}

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
		return strategy.objective_goal_node == node::Invalid_NodeID && platoons.empty();
	}

	game::Client* Troops::GetLeader() const POKEBOT_NOEXCEPT {
		assert(!strategy.leader_name.empty());
		auto leader_client = game::game.clients.Get(strategy.leader_name.c_str());
		return const_cast<game::Client*>(leader_client);
	}

	int Troops::CreatePlatoon(decltype(condition) target_condition) noexcept {
		platoons.push_back({ target_condition,  Team() });
		platoons.back().parent = this;
		return platoons.size() - 1;
	}

	void Troops::DecideStrategy(Bots* const bots, game::MapFlags map_flags) {
		/*
			Decide the new strategy based below:
				1. Offender, Defender, or Neither
				2. The important role such as a V.I.P or a bomber exists or not

			Then,
				1. Split or Stick
		*/
		if (bots->empty())
			return;

		enum class Role {
			Offender,	// The team is the offender(Terrorist in de maps, CT in cs maps).
			Defender,	// The team is the defender(Terrorist in cs maps, CT in de maps).
			Neither		// The team is neither the offender or the defender.
		} role = Role::Neither;
		
		TroopsStrategy new_strategy{};
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
					new_strategy.strategy = TroopsStrategy::Strategy::Rush_And_Rescue;
					DecideStrategyToRescueHostageSplit(bots, &new_strategy);
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
		SetNewStrategy(new_strategy);
	}

	void Troops::DecideStrategyFromRadio(Bots* const bots, const std::optional<RadioMessage>& radio_message) {
		if (bots->empty())
			return;

		TroopsStrategy new_strategy{};
		DecideSpecialStrategy(bots, &new_strategy, radio_message);
		SetNewStrategy(new_strategy);
	}

	void Troops::DecideSpecialStrategy(Bots* bots, TroopsStrategy* new_strategy, const std::optional<RadioMessage>& radio_message) {
		if (radio_message.has_value()) {
			if (radio_message->message == "#Take_Bomb") {
				assert(Team() == common::Team::T);
				new_strategy->strategy = TroopsStrategy::Strategy::Take_Backpack;
				new_strategy->leader_name.clear();
			} else {
				new_strategy->strategy = TroopsStrategy::Strategy::Follow;
				new_strategy->leader_name = radio_message->sender.data();
			}
		} else {
			if (!IsRoot()) {
				if (team == common::Team::T) {
					new_strategy->strategy = TroopsStrategy::Strategy::Follow;
					new_strategy->leader_name = Manager::Instance().Bomber_Name.data();
				}
			}
		}
	}

	void Troops::Command(std::unordered_map<util::PlayerName, Bot, util::PlayerName::Hash>* bots) {
		for (auto& individual : (*bots | std::views::filter(condition))) {
			Command(&individual.second);
		}
	}

	void Troops::Command(bot::Bot* const bot) {
		bot->ReceiveCommand(strategy);
	}

	void Troops::SetNewStrategy(const TroopsStrategy& New_Team_Strategy) noexcept {
		old_strategy = strategy;
		strategy = New_Team_Strategy;

		// Apply the strategy to platoons if they exist.
		for (int i = 0; i < platoons.size(); i++) {
			if (strategy.hostage_id.has_value()) strategy.hostage_id = i;
			platoons[i].SetNewStrategy(strategy);
		}
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
				[](const BotPair& target) -> bool { return target.second.JoinedPlatoon() == 0; }
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
					CreatePlatoon([i](const BotPair& target) -> bool { return i == target.second.JoinedPlatoon(); });
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

	void Troops::DecideStrategyToRescueHostageSplit(Bots* bots, TroopsStrategy* new_strategy) {
		new_strategy->strategy = TroopsStrategy::Strategy::Rush_And_Rescue;
		if (IsRoot()) {
			// If the troop is the root, create new platoons.
			DeleteAllPlatoon();

			auto cts = (*bots | std::views::filter(filter::ByTeam<common::Team::CT>));
			if (const size_t Number_Of_Cts = std::distance(cts.begin(), cts.end()); Number_Of_Cts > 1) {
				const size_t Number_Of_Hostages = game::game.GetNumberOfHostages();
				assert(Number_Of_Hostages > 0);

				// Found the platoons equal to the number of hostages.
				for (size_t i = 0u; i < Number_Of_Hostages; i++) {
					CreatePlatoon([i](const BotPair& target) -> bool { return i == target.second.JoinedPlatoon(); });
				}

				// Let bots join the platoons.
				const size_t Number_Of_Member_In_Squad = static_cast<size_t>(std::ceil(static_cast<common::Dec>(Number_Of_Cts) / static_cast<common::Dec>(Number_Of_Hostages)));
				auto member = cts.begin();
				for (int j = 0; j < Number_Of_Member_In_Squad && member != cts.end(); j++, member++) {
					const int Squad = j % 4;
					member->second.JoinPlatoon(Squad);
				}
				// Allow to assign hostage id for platoons.
				new_strategy->hostage_id = -1;
			} else {
				// Allow to assign hostage id for platoons.
				new_strategy->hostage_id = 1;
			}
		} else {
			// If the platoon executes the member function.
			new_strategy->hostage_id = strategy.hostage_id;
		}
	}

	void Troops::DecideStrategyToPreventRescue(Bots* bots, TroopsStrategy* new_strategy) {
		new_strategy->strategy = TroopsStrategy::Strategy::Prevent_Hostages;
		if (IsRoot()) {
			// If the troop is the root, create new platoons.
			DeleteAllPlatoon();

			auto terrorists = (*bots | std::views::filter(filter::ByTeam<common::Team::T>));
			const size_t Number_Of_Terrorists = std::distance(terrorists.begin(), terrorists.end());
			if (Number_Of_Terrorists > 1) {
				const size_t Number_Of_Goals = game::game.GetNumberOfHostages();
				assert(Number_Of_Goals > 0);
				for (int i = 0; i < Number_Of_Goals; i++) {
					CreatePlatoon([i](const BotPair& target) -> bool { return i == target.second.JoinedPlatoon(); });
				}

				const size_t Number_Of_Member_In_Squad = static_cast<size_t>(std::ceil(static_cast<common::Dec>(Number_Of_Terrorists) / static_cast<common::Dec>(Number_Of_Goals)));
				auto member = terrorists.begin();
				for (int squad = 0; squad < Number_Of_Goals; squad++) {
					for (int j = 0; j < Number_Of_Member_In_Squad && member != terrorists.end(); j++, member++) {
						member->second.JoinPlatoon(squad);
					}
				}
			} else {
				new_strategy->strategy = TroopsStrategy::Strategy::Prevent_Hostages;
				edict_t* hostage = nullptr;
				while ((hostage = common::FindEntityByClassname(hostage, "hostage_entity")) != nullptr) {
					if (auto area = node::czworld.GetNearest(hostage->v.origin); area != nullptr) {
						if (IsRoot()) {
							if (HasGoalBeenDevised(area->m_id)) {
								continue;
							}
						} else {
							if (HasGoalBeenDevisedByOtherPlatoon(area->m_id)) {
								continue;
							}
						}
						new_strategy->objective_goal_node = area->m_id;
						break;
					}
				}
			}
		}
	}
}