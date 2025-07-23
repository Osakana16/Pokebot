module;
#include "navmesh/navigation_map.h"

export module pokebot.terrain.graph.graph_base;
import pokebot.terrain.goal;
import pokebot.terrain.util;

import pokebot.game.util;
import pokebot.terrain.graph.path.path_walk;
import pokebot.terrain.graph.node.id;
import pokebot.terrain.graph.node.flag;
import pokebot.game.cs.team;

export namespace pokebot::node {
	class Graph {
	public:
		using GoalKindRange = decltype(static_cast<const std::unordered_multimap<pokebot::node::GoalKind, NodeID>>(std::unordered_multimap<pokebot::node::GoalKind, NodeID>()).equal_range(pokebot::node::GoalKind::None));

		virtual GoalKindRange GetNodeByKind(const pokebot::node::GoalKind kind) const = 0;
		virtual std::optional<HLVector> GetOrigin(const NodeID Node_ID) const = 0;
		virtual size_t GetNumberOfGoals(GoalKind) const = 0;

		virtual void FindPath(PathWalk<std::uint32_t>* const walk_routes, const Vector& Source, const Vector& Destination, const game::Team Joined_Team) = 0;
		virtual bool IsNavFileLoaded() const = 0;
		virtual bool IsSameGoal(const NodeID Node_ID, const GoalKind Goal_Kind) const = 0;
		virtual bool IsOnNode(const Vector& Position, const NodeID Target_Node_ID) const = 0;
		virtual bool HasFlag(const NodeID id, NavmeshFlag flag) const = 0;
		virtual navmesh::NavArea* GetNearest(const Vector& Destination, const float Beneath_Limit = 120.0f) const = 0;
	};
}