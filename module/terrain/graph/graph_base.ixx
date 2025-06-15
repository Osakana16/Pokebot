export module pokebot.terrain.graph: graph_base;
import pokebot.terrain.goal;
import pokebot.terrain.util;
import pokebot.terrain.graph.node;

export namespace pokebot::node {
	class Graph {
	public:
		using GoalKindRange = decltype(static_cast<const std::unordered_multimap<pokebot::node::GoalKind, NodeID>>(std::unordered_multimap<pokebot::node::GoalKind, NodeID>()).equal_range(pokebot::node::GoalKind::None));

		virtual GoalKindRange GetNodeByKind(const pokebot::node::GoalKind kind) const = 0;
		virtual std::optional<HLVector> GetOrigin(const NodeID Node_ID) const = 0;
		virtual size_t GetNumberOfGoals(GoalKind) const = 0;
	};
}