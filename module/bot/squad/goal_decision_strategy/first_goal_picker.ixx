import pokebot.bot.squad.strategy;
import pokebot.util.random;
import pokebot.terrain.goal;
import pokebot.terrain.graph;

export module pokebot.bot.squad.goal_decision_strategy:first_goal_picker;
import :decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	template<node::GoalKind goal_kind>
	class FirstGoalPicker : public DecisionStrategy {
		node::Graph* graph;
	public:
		~FirstGoalPicker() final {}

		FirstGoalPicker(node::Graph* graph_) noexcept : graph(graph_) {
			assert(graph != nullptr);
		}

		strategy::Objective Decide() final {
			strategy::Objective objective{};
			auto goal = graph->GetNodeByKind(goal_kind);
			return goal.first->second;
		}
	};
}