import pokebot.bot.squad.strategy;
import pokebot.util.random;

export module pokebot.bot.squad.goal_decision_strategy:one_goal_strategy;
import :decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	template<node::GoalKind goal_kind>
	class OneGoalStrategy : public DecisionStrategy {
		node::Graph* graph;
	public:
		~OneGoalStrategy() final {}
		
		OneGoalStrategy(node::Graph* graph_) noexcept : graph(graph_) {
			assert(graph != nullptr);
		}

		strategy::Objective Decide() final {
			strategy::Objective objective{};
			auto goal = graph->GetNodeByKind(goal_kind);
			int i = 0;
			for (auto it = goal.first; it != goal.second; it++) {
				objective = it->second;
				break;
			}
			return strategy::Objective{  };
		}
	};
}