import pokebot.bot.squad.strategy;
import pokebot.util.random;
import pokebot.terrain.graph;
import pokebot.terrain.goal;

export module pokebot.bot.squad.goal_decision_strategy:last_goal_picker;
import :decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	template<node::GoalKind goal_kind>
	class LastGoalPicker : public DecisionStrategy {
		node::Graph* graph;
	public:
		~LastGoalPicker() final {}
		
		LastGoalPicker(node::Graph* graph_) noexcept : graph(graph_) {
			assert(graph != nullptr);
		}

		strategy::Objective Decide() final {
			strategy::Objective objective{};
			auto first_goal = graph->GetNodeByKind(goal_kind);
			auto alternative_goal = first_goal.first;
			while (alternative_goal != first_goal.second) alternative_goal++;
			alternative_goal--;
			return alternative_goal->second;
		}
	};
}