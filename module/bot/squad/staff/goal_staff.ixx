module;
import std;
export module pokebot.bot.squad.staff:goal_staff;
import :squad_staff;

import pokebot.game;
import pokebot.bot.squad.strategy;
import pokebot.bot.squad.goal_decision_strategy;
import pokebot.terrain.graph.graph_base;

namespace pokebot::bot::squad::staff {	
	

	class GoalStaff : public SquadStaff {
		std::variant<goal_decision_strategy::DistanceCondition, goal_decision_strategy::FoundOrderCondition> condition{};
		strategy::Objective *objective;
		node::Graph* graph;
		game::GameBase* game;
	public:
		GoalStaff(node::Graph* graph_,
				  game::GameBase* game_,
				  game::Team team, 
				  game::MapFlags flag) : SquadStaff(team, flag), graph(graph_), game(game_){}

		GoalStaff& SetCondition(const goal_decision_strategy::DistanceCondition distance_condition) {
			this->condition = distance_condition;
			return *this;
		}

		GoalStaff& SetCondition(const goal_decision_strategy::FoundOrderCondition found_order_condition) {
			this->condition = found_order_condition;
			return *this;
		}

		[[nodiscard]] auto Resolve() {
			strategy::Objective objective{};

			goal_decision_strategy::DecisionStrategyFactory factory{};
			factory = factory.SetTeam(base_team).SetMapFlag(base_map_flags).SetGameManager(game).SetGraph(graph);
			if (std::holds_alternative<goal_decision_strategy::FoundOrderCondition>(condition)) {
				factory.SetCondition(std::get<goal_decision_strategy::FoundOrderCondition>(condition));
			} else if (std::holds_alternative<goal_decision_strategy::DistanceCondition>(condition)) {
				factory.SetCondition(std::get<goal_decision_strategy::DistanceCondition>(condition));
			}
			auto decision_strategy = factory.Create();
			return decision_strategy->Decide();
		}
	};
}