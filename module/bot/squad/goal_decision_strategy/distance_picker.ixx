import pokebot.bot.squad.strategy;
import pokebot.game.util;
import pokebot.terrain.goal;
import pokebot.terrain.graph.graph_base;

export module pokebot.bot.squad.goal_decision_strategy:distance_picker;
import :decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	template<pokebot::game::Team team, pokebot::node::GoalKind goal_kind>
	class ClosetDistancePicker : public DecisionStrategy {
		node::Graph* graph;
		
		static consteval pokebot::node::GoalKind GetTeamSpawner() {
			static_assert(team == pokebot::game::Team::T || team == pokebot::game::Team::CT);
			if constexpr (team == pokebot::game::Team::T) {
				return pokebot::node::GoalKind::Terrorist_Spawn;
			} else if constexpr (team == pokebot::game::Team::CT) {
				return pokebot::node::GoalKind::CT_Spawn;
			}
		}
	public:
		~ClosetDistancePicker() final {}

		ClosetDistancePicker(node::Graph* graph_) noexcept : graph(graph_) {
			assert(graph != nullptr);
		}

		strategy::Objective Decide() final {
			pokebot::node::NodeID result = pokebot::node::Invalid_NodeID;
			auto spawn = graph->GetNodeByKind(GetTeamSpawner());
			auto first_spawn_node_id = spawn.first->second;
			auto spawn_origin = graph->GetOrigin(first_spawn_node_id);

			float min_distance = std::numeric_limits<float>::max();
			auto goal = graph->GetNodeByKind(goal_kind);
			for (auto it = goal.first; it != goal.second; it++) {
				auto goal_origin = graph->GetOrigin(it->second);

				float new_distance = pokebot::game::Distance(*reinterpret_cast<Vector*>(&*goal_origin), *reinterpret_cast<Vector*>(&*spawn_origin));
				if (new_distance < min_distance) {
					min_distance = new_distance;
					result = it->second;
				}
			}
			return result;
		}
	};

	template<pokebot::game::Team team, pokebot::node::GoalKind goal_kind>
	class FurthestDistancePicker : public DecisionStrategy {
		node::Graph* graph;

		static constexpr pokebot::node::GoalKind GetTeamSpawner() {
			if constexpr (team == pokebot::game::Team::T) {
				return pokebot::node::GoalKind::Terrorist_Spawn;
			} else if constexpr (team == pokebot::game::Team::CT) {
				return pokebot::node::GoalKind::CT_Spawn;
			}
		}
	public:
		~FurthestDistancePicker() final {}

		FurthestDistancePicker(node::Graph* graph_) noexcept : graph(graph_) {
			assert(graph != nullptr);
		}

		strategy::Objective Decide() final {
			pokebot::node::NodeID result = pokebot::node::Invalid_NodeID;
			auto spawn = graph->GetNodeByKind(GetTeamSpawner());
			auto first_spawn_node_id = spawn.first->second;
			auto spawn_origin = graph->GetOrigin(first_spawn_node_id);

			float max_distance = std::numeric_limits<float>::min();
			auto goal = graph->GetNodeByKind(goal_kind);
			for (auto it = goal.first; it != goal.second; it++) {
				auto goal_origin = graph->GetOrigin(it->second);

				float new_distance = pokebot::game::Distance(*reinterpret_cast<Vector*>(&*goal_origin), *reinterpret_cast<Vector*>(&*spawn_origin));
				if (new_distance > max_distance) {
					max_distance = new_distance;
					result = it->second;
				}
			}
			return result;
		}
	};
}