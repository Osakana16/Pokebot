module;
import std;

export module pokebot.bot.squad.goal_decision_strategy:factory;
import :decision_strategy;

import :one_goal_strategy;
import :first_goal_picker;
import :last_goal_picker;
import :distance_picker;
import :multiple_goal_strategy;
import :hostage_rescue;

import pokebot.game.game_manager_base;
import pokebot.terrain.graph.graph_base;
import pokebot.game.util;

import pokebot.bot.squad.strategy;
import pokebot.bot.squad.util;

namespace pokebot::bot::squad::goal_decision_strategy {
	export enum class DistanceCondition {
		Closet,
		Furthest
	};

	export enum class FoundOrderCondition {
		First,
		Last
	};

	export class DecisionStrategyFactory {
		std::variant<DistanceCondition, FoundOrderCondition> condition{};

		std::optional<game::Team> team{};
		std::optional<game::MapFlags> map_flag{};
		node::Graph* graph{};
		pokebot::game::GameBase* game{};

		std::optional<int> objective_choice{};
	public:
		DecisionStrategyFactory() {}

		DecisionStrategyFactory& SetTeam(const game::Team Base_Team) {
			assert(Base_Team != game::Team::Spector && "Spector is not supported.");
			assert(Base_Team != game::Team::Random && "Random is not supported.");
			team = Base_Team;
			return *this;
		}

		DecisionStrategyFactory& SetMapFlag(const game::MapFlags Base_Map_Flags) {
			map_flag = Base_Map_Flags;
			return *this;
		}

		DecisionStrategyFactory& SetLeaderName(const pokebot::util::PlayerName) {
			return *this;
		}

		DecisionStrategyFactory& SetGameManager(pokebot::game::GameBase* game) {
			assert(game != nullptr);
			this->game = game;
			return *this;
		}

		DecisionStrategyFactory& SetGraph(node::Graph* graph) {
			assert(graph != nullptr);
			this->graph = graph;
			return *this;
		}

		DecisionStrategyFactory& SetChoice(const int objective_choice) {
			this->objective_choice = objective_choice;
			return *this;
		}

		DecisionStrategyFactory& SetCondition(const DistanceCondition distance_condition) {
			this->condition = distance_condition;
			return *this;
		}

		DecisionStrategyFactory& SetCondition(const FoundOrderCondition found_order_condition) {
			this->condition = found_order_condition;
			return *this;
		}

		std::unique_ptr<DecisionStrategy> Create() {
			assert(team.has_value());
			const auto My_Role = util::JudgeRole(*team, *map_flag);

			std::unique_ptr<DecisionStrategy> result = nullptr;
			switch (*team) {
				case game::Team::T:
				{
					if (bool(*map_flag & game::MapFlags::Demolition)) {
						assert(!condition.valueless_by_exception());
						if (std::holds_alternative<FoundOrderCondition>(condition)) {
							switch (std::get<FoundOrderCondition>(condition)) {
								case FoundOrderCondition::First:
									result = std::make_unique<FirstGoalPicker<node::GoalKind::Bombspot>>(graph);
									break;
								case FoundOrderCondition::Last:
									result = std::make_unique<LastGoalPicker<node::GoalKind::Bombspot>>(graph);
									break;
								default:
									assert(false);
							}
						} else if (std::holds_alternative<DistanceCondition>(condition)) {
							switch (std::get<DistanceCondition>(condition)) {
								case DistanceCondition::Closet:
									result = std::make_unique<ClosetDistancePicker<game::Team::T, node::GoalKind::Bombspot>>(graph);
									break;
								case DistanceCondition::Furthest:
									result = std::make_unique<FurthestDistancePicker<game::Team::T, node::GoalKind::Bombspot>>(graph);
									break;
								default:
									assert(false);
							}
						}
					} else if (bool(*map_flag & game::MapFlags::HostageRescue)) {
						result = std::make_unique<RescueHostageStrategy>(game);
					} else if (bool(*map_flag & game::MapFlags::Assassination)) {
						result = std::make_unique<OneGoalStrategy<node::GoalKind::Vip_Safety>>(graph);
					} else if (bool(*map_flag & game::MapFlags::Escape)) {
						result = std::make_unique<OneGoalStrategy<node::GoalKind::Escape_Zone>>(graph);
					} else {
						assert(false && "The scenario is not supported.");
					}
					break;
				}
				case game::Team::CT:
				{
					if (bool(*map_flag & game::MapFlags::Demolition)) {
						assert(!condition.valueless_by_exception());
						if (std::holds_alternative<FoundOrderCondition>(condition)) {
							switch (std::get<FoundOrderCondition>(condition)) {
								case FoundOrderCondition::First:
									result = std::make_unique<FirstGoalPicker<node::GoalKind::Bombspot>>(graph);
									break;
								case FoundOrderCondition::Last:
									result = std::make_unique<LastGoalPicker<node::GoalKind::Bombspot>>(graph);
									break;
							}
						} else if (std::holds_alternative<DistanceCondition>(condition)) {
							switch (std::get<DistanceCondition>(condition)) {
								case DistanceCondition::Closet:
									result = std::make_unique<ClosetDistancePicker<game::Team::CT, node::GoalKind::Bombspot>>(graph);
									break;
								case DistanceCondition::Furthest:
									result = std::make_unique<FurthestDistancePicker<game::Team::CT, node::GoalKind::Bombspot>>(graph);
									break;
							}
						}
					} else if (bool(*map_flag & game::MapFlags::HostageRescue)) {
						result = std::make_unique<RescueHostageStrategy>(game);
					} else if (bool(*map_flag & game::MapFlags::Assassination)) {
						result = std::make_unique<OneGoalStrategy<node::GoalKind::Vip_Safety>>(graph);
					} else if (bool(*map_flag & game::MapFlags::Escape)) {
						result = std::make_unique<OneGoalStrategy<node::GoalKind::Escape_Zone>>(graph);
					} else {
						assert(false && "The scenario is not supported.");
					}
					break;
				}
				default:
				{
					assert(false);
				}
			}
			return result;
		}
	};
}