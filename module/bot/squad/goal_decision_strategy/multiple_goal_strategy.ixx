import pokebot.bot.squad.strategy;

export module pokebot.bot.squad.goal_decision_strategy:multiple_goal_strategy;
import :decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	template<node::GoalKind goal_kind>
	class MultipleGoalStrategy : public DecisionStrategy {
		strategy::Objective* objective{};
		node::Graph* graph;
	public:
		~MultipleGoalStrategy() final {}
		MultipleGoalStrategy(strategy::Objective *objective_, node::Graph* graph_) : objective(objective_), graph(graph_) {}

		void Decide() final {
			auto goal = graph->GetNodeByKind(goal_kind);
			int i = 0;
			for (auto it = goal.first; it != goal.second; it++) {
				*objective = it->second;
			}
		}
	};
}