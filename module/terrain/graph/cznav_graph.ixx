#include "navmesh/navigation_map.h"
export module pokebot.terrain.graph.cznav_graph;
import pokebot.terrain.graph.danger;
import pokebot.terrain.graph.graph_base;
import pokebot.terrain.graph.path;
import pokebot.terrain.graph.node.id;
import pokebot.terrain.graph.node.flag;

import pokebot.plugin.observables;
import pokebot.engine;
import pokebot.common.event_handler;
import pokebot.game.util;
import pokebot.util;

export namespace pokebot::node {
	// Compatibility with ZBot navmesh.
	class CZBotGraph : public Graph {
		std::unordered_multimap<GoalKind, NodeID> goals{};
		game::Array<Danger, 2> danger;
		bool is_nav_loaded{};
	public:
		CZBotGraph(plugin::Observables*, engine::Observables*);

		navmesh::NavigationMap navigation_map{};
		std::vector<Route<navmesh::NavArea*>> routes{};

		navmesh::NavArea* GetNearest(const Vector& Destination, const float Beneath_Limit = 120.0f) const POKEBOT_NOEXCEPT {
			return navigation_map.GetNavArea(&Destination, Beneath_Limit);
		}

		void FindPath(PathWalk<std::uint32_t>* const walk_routes, const Vector& Source, const Vector& Destination, const game::Team Joined_Team) override {
			static CZBotGraph *self;
			self = this;

			const int Joined_Team_Index = static_cast<int>(Joined_Team) - 1;
			assert(Joined_Team_Index >= 0 && Joined_Team_Index <= 1);
			auto source = navigation_map.GetNavArea(&Source);
			if (source == nullptr) {
				return;
			}

			auto destination = navigation_map.GetNavArea(&Destination);
			if (destination == nullptr) {
				return;
			}
			auto start_node_id = source->m_id;
			auto end_node_id = destination->m_id;
			if (start_node_id == end_node_id)
				return;

			routes.clear();
			class FGreater {
			public:
				bool operator ()(const NodeID a_id, const NodeID b_id) {
					return self->routes.at(a_id).f > self->routes.at(b_id).f;
				}
			};
			routes.resize(navmesh::NavArea::m_nextID);
			if (navigation_map.GetNavAreaByID(end_node_id) == nullptr) {
				return;
			}

			// Self Node-ID/Parent Node-ID
			std::unordered_map<NodeID, NodeID> found_path{ { start_node_id, Invalid_NodeID } };

			NodeID last_opened_node_id{};
			std::unordered_set<NodeID> closed_list{};
			std::priority_queue<
				NodeID,
				std::vector<NodeID>,
				FGreater
			> route_queue{};
			route_queue.push(start_node_id);

			bool has_another_jump{};
			routes[start_node_id].state = RouteState::Open;
			routes[start_node_id].f = pokebot::game::Distance(source->m_center, destination->m_center);
			routes[start_node_id].g = game::Distance(source->m_center, destination->m_center);
			while (!route_queue.empty()) {
				NodeID current_node_id = route_queue.top();
				route_queue.pop();

				if (current_node_id == end_node_id) {
					do {
						assert(walk_routes->Size() <= navmesh::NavArea::m_nextID);
						if (auto it = danger[Joined_Team_Index].number_of_reference.find(current_node_id); it != danger[Joined_Team_Index].number_of_reference.end()) {
							it->second++;
						} else {
							danger[Joined_Team_Index].number_of_reference[current_node_id] = 1;
						}
						walk_routes->PushFront(current_node_id);
						current_node_id = routes[current_node_id].parent;
					} while (current_node_id != start_node_id);
					walk_routes->PushFront(current_node_id);
					return;
				}

				auto current_route = &routes.at(current_node_id);
				if (current_route->state != RouteState::Open) {
					continue;
				}

				current_route->state = RouteState::Closed;
				for (auto& direction : navigation_map.GetNavAreaByID(current_node_id)->m_connect) {
					for (auto& connection : direction) {
						assert(connection.area != nullptr);
						auto near_route = &routes.at(connection.area->m_id);
						auto current_node = navigation_map.GetNavAreaByID(current_node_id);
						assert(current_node != nullptr);
						auto near_node = navigation_map.GetNavAreaByID(connection.area->m_id);
						assert(near_node != nullptr);

						float base_cost{};
#if 1
						if (bool(current_node->m_attributeFlags & navmesh::NavAttributeType::NAV_JUMP)) {
							base_cost += 9000.0f * (has_another_jump ? 3 : 1);
							has_another_jump = true;
						}
						if (bool(current_node->m_attributeFlags & navmesh::NavAttributeType::NAV_CROUCH)) {
							base_cost += 3000.0f;
						}
						base_cost += 2000.0f * danger[Joined_Team_Index].number_of_reference[current_node_id];
#endif
						float h = game::Distance(source->m_center, current_node->m_center) + game::Distance(current_node->m_center, destination->m_center);
						float g = current_route->g + std::abs(near_route->f - h) + base_cost;
						float f = g + h;
						if (near_route->state == RouteState::New || near_route->f > f) {
							near_route->parent = current_node_id;
							near_route->state = RouteState::Open;

							near_route->f = f;
							near_route->g = g;

							route_queue.push(connection.area->m_id);
						}
					}
				}
			}
		}

		bool IsNavFileLoaded() const noexcept {
			return is_nav_loaded;
		}


		size_t GetNumberOfGoals(const GoalKind Kind) const noexcept {
			size_t number{};
			auto goals = GetNodeByKind(Kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				number++;
			}
			return number;
		}

		bool IsSameGoal(const NodeID Node_ID, const GoalKind Goal_Kind) const POKEBOT_NOEXCEPT {
			auto goals = GetNodeByKind(Goal_Kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				if (Node_ID == goal->second) {
					return true;
				}
			}
			return false;
		}

		bool IsOnNode(const Vector& Position, const NodeID Target_Node_ID) const POKEBOT_NOEXCEPT {
			auto area = GetNearest(Position);
			return (area != nullptr && area->m_id == Target_Node_ID);
		}

		Graph::GoalKindRange GetNodeByKind(const GoalKind kind) const POKEBOT_NOEXCEPT {

			return goals.equal_range(kind);
		}


		std::optional<HLVector> GetOrigin(const NodeID Node_ID) const POKEBOT_NOEXCEPT {
			if (auto area = navigation_map.GetNavAreaByID(Node_ID); area != nullptr) {
				return HLVector{ .x = area->m_center.x, .y = area->m_center.y, .z = area->m_center.z };
			} else {
				return std::nullopt;
			}
		}


		bool HasFlag(const NodeID id, NavmeshFlag flag) const POKEBOT_NOEXCEPT {
			auto area = navigation_map.GetNavAreaByID(id);
			return area != nullptr && bool(area->m_attributeFlags & static_cast<navmesh::NavAttributeType>(flag));
		}
	};
}