#include <memory>
export module pokebot.terrain.graph: waypoint_graph;
import pokebot.terrain.graph.node;
import pokebot.terrain.graph.path;
import pokebot.terrain.goal;

import pokebot.game.util;
import pokebot.util.tracer;

namespace pokebot::node {
	template<typename T>
	constexpr T World_Size = 8192;
	constexpr size_t Split_Size = 256;
	constexpr size_t Tree_Size = World_Size<size_t> / Split_Size;

	size_t PointAsIndex(const float Pos) POKEBOT_NOEXCEPT {
		return static_cast<size_t>(Pos / Split_Size) + 16;
	}

	class WaypointGraph {
		friend class FGreater;

		std::unordered_map<NodeID, std::shared_ptr<Point>> nodes{};
		std::unordered_multimap<GoalKind, NodeID> goals{};
		std::vector<NodeID> points_tree[Tree_Size][Tree_Size][Tree_Size]{};

		std::vector<Route<NodeID>> routes{};
	public:

		NodeID TryToConnect(const NodeID Node_ID) {
			auto& new_point = nodes.at(Node_ID);
			for (auto closed_point_id : nodes) {
				auto& closed_point = nodes.at(closed_point_id.first);
				if (new_point == closed_point)
					continue;

				if (game::Distance(new_point->Origin(), closed_point->Origin()) <= Range<float> *1.95) {
					util::Tracer tracer{};
					tracer.MoveStart(new_point->Origin()).MoveDest(closed_point->Origin());
					if (tracer.TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit())
						continue;

					new_point->AddConnection(closed_point_id.first);
					closed_point->AddConnection(Node_ID);
				}
			}
			return Node_ID;
		}

		void Clear() {
			nodes.clear();
			for (int z = 0; z < Tree_Size; z++) {
				for (int y = 0; y < Tree_Size; y++) {
					for (int x = 0; x < Tree_Size; x++) {
						points_tree[z][y][x].clear();
					}
				}
			}
		}

		void Remove(NodeID point_id) {
			nodes.erase(point_id);
		}

		std::shared_ptr<Point> GetNode(NodeID point_id) {
			if (auto point_iterator = nodes.find(point_id); point_iterator != nodes.end())
				return point_iterator->second;
			else
				return nullptr;
		}

		NodeID Add(const Vector& Position, const GoalKind Kind) {
			NodeID current_node_id = GetNearest(Position);
			if (current_node_id == Invalid_NodeID) {
				const size_t X = PointAsIndex(Position.x),
					Y = PointAsIndex(Position.y),
					Z = PointAsIndex(Position.z);

				NodeID new_point_id = nodes.size();
				nodes.insert({ new_point_id, std::make_shared<Point>(Position) });
				if (Kind != GoalKind::None)
					goals.insert({ Kind, new_point_id });

				points_tree[Z][Y][X].push_back(new_point_id);
				return new_point_id;
			}
			TryToConnect(current_node_id);
			return Invalid_NodeID;
		}

		bool IsSameGoal(const NodeID Node_ID, const GoalKind Goal_Kind) const POKEBOT_NOEXCEPT {
			auto goals = GetGoal(node::GoalKind::Bombspot);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				if (Node_ID == goal->second) {
					return true;
				}
			}
			return false;
		}

		bool IsOnNode(const Vector& Position, const NodeID Target_Node_ID) const POKEBOT_NOEXCEPT {
			return (GetNearest(Position) == Target_Node_ID);
		}

		void Draw() {
#if 0
			auto host = const_cast<edict_t*>(game::game.host.AsEdict());
			float nearest_distance = std::numeric_limits<float>::max();
			for (int i = 0; i < nodes.size(); i++) {
				auto& target_node = GetNode(i);
				float distance = (target_node->Origin() - host->v.origin).Length();
				if (distance < 512.0f && ((entity::IsVisible(host, target_node->Origin()) && entity::InViewCone(host, target_node->Origin()) || !game::Client::IsDead(host) || distance < 128.0f))) {
					if (distance < nearest_distance) {
						nearest_distance = distance;
					}

					if (target_node->time + 0.8f < gpGlobals->time) {
						pokebot::common::Draw(host, target_node->Origin() - Vector(0, 0, 60), target_node->Origin() + Vector(0, 0, 60), 10 + 1, 0, common::Color{ .r = 255, .g = 255, .b = 255 }, 250, 0, 10);
						target_node->time = gpGlobals->time;
					}
				}
			}
#endif
		}

		void AddBasic() {
			auto MoveOriginOnGround = [](Vector* const origin) POKEBOT_NOEXCEPT{
				util::Tracer tracer{};
				tracer.MoveStart(*origin);
				tracer.MoveDest(Vector(origin->x, origin->y, -9999));
				tracer.TraceHull(util::Tracer::Monsters::Ignore, util::Tracer::HullType::Human, nullptr);
				if (tracer.IsHit() && std::string(STRING(tracer.pHit->v.classname)) == "worldspawn") {
					origin->z = tracer.vecEndPos.z;
				}
			};

			edict_t* entity = nullptr;
			while ((entity = game::FindEntityByClassname(entity, "info_player_start")) != nullptr) {
				Vector origin = entity->v.origin;
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::CT_Spawn);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL #2 - Terrorist Spawn points.
			while ((entity = game::FindEntityByClassname(entity, "info_player_deathmatch")) != nullptr) {
				Vector origin = entity->v.origin;
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Terrorist_Spawn);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL #3 - Hostage rescue zone
			while ((entity = game::FindEntityByClassname(entity, "func_hostage_rescue")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Rescue_Zone);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// rescue zone can also be an entity of info_hostage_rescue
			while ((entity = game::FindEntityByClassname(entity, "info_hostage_rescue")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Rescue_Zone);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL #4 - Bombspot zone
			// Bomb spot
			while ((entity = game::FindEntityByClassname(entity, "func_bomb_target")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Bombspot);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			while ((entity = game::FindEntityByClassname(entity, "info_bomb_target")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Bombspot);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL  #6 - VIP (this is the 'starting' position) (EVY)
			while ((entity = game::FindEntityByClassname(entity, "info_vip_start")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::CT_Spawn);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL  #7 - VIP safety (this is the 'rescue' position) (EVY)
			while ((entity = game::FindEntityByClassname(entity, "func_vip_safetyzone")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Vip_Safety);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL  #8 - Escape zone for es_ (EVY)
			while ((entity = game::FindEntityByClassname(entity, "func_escapezone")) != nullptr) {
				Vector origin = game::VecBModelOrigin(entity);
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::Escape_Zone);
				auto point = GetNode(point_id);
				if (point != nullptr) {
					point->AddFlag(NodeFlag::Goal);
				}
			}

			// GOAL  #9 - Escape zone for es_ (EVY)
			while ((entity = game::FindEntityByClassname(entity, "hostage_entity")) != nullptr) {
				Vector origin = entity->v.origin;
				MoveOriginOnGround(&origin);
				NodeID point_id = Add(origin, GoalKind::None);
			}
		}

		void OnMapLoaded() {
			Clear();
		}

		void OnNewRound() {
			if (!Load()) {
				AddBasic();
			}
			Save();
		}

		void Remove(const Vector& Position) {
			if (auto point_id = GetNearest(Position); point_id != Invalid_NodeID) {
				Remove(point_id);
			}
		}

		void FindPath(PathWalk<NodeID>* const walk_routes, const Vector& Start, const Vector& Goal) {
			FindPath(walk_routes, GetNearest(Start), GetNearest(Goal));
		}


		void FindPath(PathWalk<NodeID>* const walk_routes, const NodeID start_node_id, const NodeID end_node_id) {
#if 0
			routes.clear();
			class FGreater {
			public:
				bool operator ()(const NodeID a_id, const NodeID b_id) {
					auto a = std::static_pointer_cast<Point>(world.GetNode(a_id));
					auto b = std::static_pointer_cast<Point>(world.GetNode(b_id));

					return world.routes.at(a_id).f > world.routes.at(b_id).f;
				}
			};
			routes.resize(nodes.size());
			if (GetNode(end_node_id) == nullptr)
				return;

			auto start_node = GetNode(start_node_id);
			auto end_node = GetNode(end_node_id);

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

			routes[start_node_id].state = RouteState::Open;
			routes[start_node_id].f = game::Distance(start_node->Origin(), end_node->Origin());
			routes[start_node_id].g = game::Distance(start_node->Origin(), end_node->Origin());
			while (!route_queue.empty()) {
				NodeID current_node_id = route_queue.top();
				route_queue.pop();
				if (current_node_id == end_node_id) {
					do {
						walk_routes->PushFront(current_node_id);
						current_node_id = routes[current_node_id].parent;
					} while (current_node_id != Invalid_NodeID);
					return;
				}

				auto current_route = &routes[current_node_id];
				if (current_route->state != RouteState::Open) {
					continue;
				}

				current_route->state = RouteState::Closed;
				auto connections = GetNode(current_node_id)->GetConnections();
				for (auto near_node_id = connections.first; near_node_id != connections.second; near_node_id++) {
					auto near_route = &routes.at(*near_node_id);

					auto current_node = GetNode(current_node_id);
					auto near_node = GetNode(*near_node_id);

					float h = game::Distance(start_node->Origin(), current_node->Origin()) + game::Distance(current_node->Origin(), end_node->Origin());
					float g = current_route->g + std::abs(near_route->f - h);
					float f = g + h;
					if (near_route->state == RouteState::New || near_route->f > f) {
						near_route->parent = current_node_id;
						near_route->state = RouteState::Open;

						near_route->f = f;
						near_route->g = g;

						route_queue.push(*near_node_id);
					}
				}
			}
#endif
		}


		decltype(static_cast<const decltype(goals)>(goals).equal_range(GoalKind::None)) GetGoal(const GoalKind kind) const POKEBOT_NOEXCEPT {
			return goals.equal_range(kind);
		}

		NodeID GetNearest(const Vector& Destination) const POKEBOT_NOEXCEPT {
			NodeID result = Invalid_NodeID;
			const size_t X = PointAsIndex(Destination.x),
				Y = PointAsIndex(Destination.y),
				Z = PointAsIndex(Destination.z);

			auto FindNodeByTree = [&](const std::vector<NodeID>* const points) {
				NodeID result = Invalid_NodeID;
				for (auto point_id : *points) {
					auto& point = nodes.at(point_id);
					if (game::Distance(Destination, point->Origin()) <= Range<float>) {
						result = point_id;
						break;
					}
				}
				return result;
			};

			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					for (int z = -1; z <= 1; z++) {
						if (Z + z < 0 || Z + z >= Tree_Size ||
							X + x < 0 || X + x >= Tree_Size ||
							Y + y < 0 || Y + y >= Tree_Size) {
							continue;
						}
						result = FindNodeByTree(&points_tree[Z + z][Y + y][X + x]);
						if (result != Invalid_NodeID)
							goto end_search;
					}
				}
			}
		end_search:
			return result;
		}

		bool Load() {
#if 0
			char mod[50];
			g_engfuncs.pfnGetGameDir(mod);

			std::filesystem::path waypoint_path = std::format("{}/pokebot/data/waypoint/{}.pkn", mod, game::ToString(gpGlobals->mapname));

			std::ifstream ifs{ waypoint_path, std::ios_base::in | std::ios_base::binary };
			if (ifs.is_open()) {
				char header[20];
				ifs.read(header, sizeof(header));

				// - Nodes -
				{
					size_t number_of_nodes{};
					ifs.read(reinterpret_cast<char*>(&number_of_nodes), sizeof(number_of_nodes));

					for (size_t i = 0; i < number_of_nodes; i++) {
						NodeID id{};
						ifs.read(reinterpret_cast<char*>(&id), sizeof(id));
						auto point = std::make_shared<Point>(Point{ {} });
						point->Read(&ifs);
						nodes.insert({ id, point });
						points_tree[PointAsIndex(point->Origin().z)][PointAsIndex(point->Origin().y)][PointAsIndex(point->Origin().x)].push_back(id);
					}
				}

				// - Goals -
				{
					size_t number_of_goals{};
					ifs.read(reinterpret_cast<char*>(&number_of_goals), sizeof(number_of_goals));

					std::pair<GoalKind, NodeID> goal{};
					for (size_t i = 0; i < number_of_goals; i++) {
						ifs.read(reinterpret_cast<char*>(&goal.second), sizeof(goal.second));
						ifs.read(reinterpret_cast<char*>(&goal.first), sizeof(goal.first));
						goals.insert(goal);
					}
				}
				return true;
			}
#endif
			return false;
		}

		bool Save() {
#if 0
			char mod[50];
			g_engfuncs.pfnGetGameDir(mod);

			std::filesystem::path waypoint_path = std::format("{}/pokebot/data/waypoint/{}.pkn", mod, game::ToString(gpGlobals->mapname));
			std::filesystem::create_directories(waypoint_path.parent_path());

			std::ofstream ofs{ waypoint_path, std::ios_base::out | std::ios_base::binary };
			if (ofs.is_open()) {
				constexpr const char Header[20] = "POKEBOT_00000000000";
				ofs.write(Header, sizeof(Header));

				// - Nodes -
				{
					const size_t Number_Of_Nodes = nodes.size();
					ofs.write(reinterpret_cast<const char*>(&Number_Of_Nodes), sizeof(Number_Of_Nodes));
					for (auto& node : nodes) {
						ofs.write(reinterpret_cast<const char*>(&node.first), sizeof(node.first));
						std::static_pointer_cast<Point>(node.second)->Write(&ofs);
					}
				}

				// - Goals -
				{
					const size_t Number_Of_Goals = goals.size();
					ofs.write(reinterpret_cast<const char*>(&Number_Of_Goals), sizeof(Number_Of_Goals));
					for (auto goal_kind : All_Goal_List) {
						auto goals = GetGoal(goal_kind);
						for (auto goal = goals.first; goal != goals.second; goal++) {
							ofs.write(reinterpret_cast<const char*>(&goal->second), sizeof(goal->second));
							ofs.write(reinterpret_cast<const char*>(&goal->first), sizeof(goal->first));
						}
					}
				}
				return true;
			}
#endif
			return false;
		}

		Vector GetOrigin(const NodeID Node_ID) const POKEBOT_NOEXCEPT {
			return (Node_ID != Invalid_NodeID ? nodes.at(Node_ID)->Origin() : Vector(9999, 9999, 9999));
		}

	};
}