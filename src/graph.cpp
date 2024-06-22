#include "graph.hpp"
#include <unordered_set>
#include <fstream>

namespace pokebot::node {
	void Point::Write(std::ofstream* const ofs) {
		ofs->write(reinterpret_cast<const char*>(&point), sizeof(point));
		ofs->write(reinterpret_cast<const char*>(&flag), sizeof(flag));
		size_t size = connections.size();
		ofs->write(reinterpret_cast<const char*>(&size), sizeof(size));
		for (auto connection : connections) {
			ofs->write(reinterpret_cast<const char*>(&connection), sizeof(connection));
		}
	}

	void Point::Read(std::ifstream* const ifs) {
		ifs->read(reinterpret_cast<char*>(&point), sizeof(point));
		ifs->read(reinterpret_cast<char*>(&flag), sizeof(flag));
		size_t size{};
		ifs->read(reinterpret_cast<char*>(&size), sizeof(size));
		for (size_t i = 0; i < size; i++) {
			NodeID id{};
			ifs->read(reinterpret_cast<char*>(&id), sizeof(id));
			connections.insert(id);
		}
	}

	const Vector& Point::Origin() const noexcept {
		return point;
	}

	std::pair<float, float> Point::Length() const noexcept {
		return std::make_pair(Range<float>, Range<float>);
	}

	size_t PointAsIndex(const float Pos) noexcept {
		return static_cast<size_t>(Pos / Split_Size) + 16;
	}

	NodeID Graph::TryToConnect(const NodeID Node_ID) {
		auto& new_point = nodes.at(Node_ID);			
		for (auto closed_point_id : nodes) {
			auto& closed_point = nodes.at(closed_point_id.first);
			if (new_point == closed_point)
				continue;

			if (common::Distance(new_point->Origin(), closed_point->Origin()) <= Range<float> *1.95) {
				common::Tracer tracer{};
				tracer.MoveStart(new_point->Origin()).MoveDest(closed_point->Origin());
				if (tracer.TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Ignore, nullptr).IsHit())
					continue;

				new_point->AddConnection(closed_point_id.first);
				closed_point->AddConnection(Node_ID);
			}
		}
		return Node_ID;
	}

	void Graph::Clear() {
		nodes.clear();
		for (int z = 0; z < Tree_Size; z++) {
			for (int y = 0; y < Tree_Size; y++) {
				for (int x = 0; x < Tree_Size; x++) {
					points_tree[z][y][x].clear();
				}
			}
		}
	}

	void Graph::Remove(NodeID point_id) {
		nodes.erase(point_id);
	}

	std::shared_ptr<Node> Graph::GetNode(NodeID point_id) {
		if (auto point_iterator = nodes.find(point_id); point_iterator != nodes.end())
			return point_iterator->second;
		else
			return nullptr;
	}

	NodeID Graph::Add(const Vector& Position, const GoalKind Kind) {
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

	
	bool Graph::IsSameGoal(const NodeID Node_ID, const GoalKind Goal_Kind) const noexcept {
		auto goals = GetGoal(node::GoalKind::Bombspot);
		for (auto goal = goals.first; goal != goals.second; goal++) {
			if (Node_ID == goal->second) {
				return true;
			}
		}
		return false;
	}

	bool Graph::IsOnNode(const Vector& Position, const NodeID Target_Node_ID) const noexcept {
		return (GetNearest(Position) == Target_Node_ID);
	}

	void Graph::Draw() {
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
	}

	void Graph::AddBasic() {
		auto MoveOriginOnGround = [](Vector* const origin) noexcept {
			common::Tracer tracer{};
			tracer.MoveStart(*origin);
			tracer.MoveDest(Vector(origin->x, origin->y, -9999));
			tracer.TraceHull(common::Tracer::Monsters::Ignore, common::Tracer::HullType::Human, nullptr);
			if (tracer.IsHit() && std::string(STRING(tracer.pHit->v.classname)) == "worldspawn") {
				origin->z = tracer.vecEndPos.z;
			}
		};

		edict_t* entity = nullptr;
		while ((entity = common::FindEntityByClassname(entity, "info_player_start")) != nullptr) {
			Vector origin = entity->v.origin;
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::CT_Spawn);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL #2 - Terrorist Spawn points.
		while ((entity = common::FindEntityByClassname(entity, "info_player_deathmatch")) != nullptr) {
			Vector origin = entity->v.origin;
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Terrorist_Spawn);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL #3 - Hostage rescue zone
		while ((entity = common::FindEntityByClassname(entity, "func_hostage_rescue")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Rescue_Zone);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// rescue zone can also be an entity of info_hostage_rescue
		while ((entity = common::FindEntityByClassname(entity, "info_hostage_rescue")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Rescue_Zone);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL #4 - Bombspot zone
		// Bomb spot
		while ((entity = common::FindEntityByClassname(entity, "func_bomb_target")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Bombspot);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		while ((entity = common::FindEntityByClassname(entity, "info_bomb_target")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Bombspot);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL  #6 - VIP (this is the 'starting' position) (EVY)
		while ((entity = common::FindEntityByClassname(entity, "info_vip_start")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::CT_Spawn);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL  #7 - VIP safety (this is the 'rescue' position) (EVY)
		while ((entity = common::FindEntityByClassname(entity, "func_vip_safetyzone")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Vip_Safety);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL  #8 - Escape zone for es_ (EVY)
		while ((entity = common::FindEntityByClassname(entity, "func_escapezone")) != nullptr) {
			Vector origin = common::VecBModelOrigin(entity);
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::Esacpe_Zone);
			auto point = GetNode(point_id);
			if (point != nullptr) {
				point->AddFlag(NodeFlag::Goal);
			}
		}

		// GOAL  #9 - Escape zone for es_ (EVY)
		while ((entity = common::FindEntityByClassname(entity, "hostage_entity")) != nullptr) {
			Vector origin = entity->v.origin;
			MoveOriginOnGround(&origin);
			NodeID point_id = Add(origin, GoalKind::None);
		}
	}

	void Graph::OnMapLoaded() {
		Clear();
	}

	void Graph::OnNewRound() {
		if (!Load()) {
			AddBasic();
		}
		Save();
	}

	void Graph::Remove(const Vector& Position) {
		if (auto point_id = GetNearest(Position); point_id != Invalid_NodeID) {
			Remove(point_id);
		}
	}

	void Graph::FindPath(PathWalk* const walk_routes, const Vector& Start, const Vector& Goal) {
		FindPath(walk_routes, GetNearest(Start), GetNearest(Goal));
	}


	void Graph::FindPath(PathWalk* const walk_routes, const NodeID start_node_id, const NodeID end_node_id) {
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
		routes[start_node_id].f = common::Distance(start_node->Origin(), end_node->Origin());
		routes[start_node_id].g = common::Distance(start_node->Origin(), end_node->Origin());
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

				float h = common::Distance(start_node->Origin(), current_node->Origin()) + common::Distance(current_node->Origin(), end_node->Origin());
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
		return;
	}


	decltype(static_cast<const decltype(Graph::goals)>(Graph::goals).equal_range(GoalKind::None)) Graph::GetGoal(const GoalKind kind) const noexcept {
		return goals.equal_range(kind);
	}

	NodeID Graph::GetNearest(const Vector& Destination) const noexcept {
		NodeID result = Invalid_NodeID;
		const size_t X = PointAsIndex(Destination.x),
			Y = PointAsIndex(Destination.y),
			Z = PointAsIndex(Destination.z);

		auto FindNodeByTree = [&](const std::vector<NodeID>* const points) {
			NodeID result = Invalid_NodeID;
			for (auto point_id : *points) {
				auto& point = nodes.at(point_id);
				if (common::Distance(Destination, point->Origin()) <= Range<float>) {
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

#include <filesystem>

	bool Graph::Load() {
		char mod[50];
		g_engfuncs.pfnGetGameDir(mod);

		std::filesystem::path waypoint_path = std::format("{}/pokebot/data/waypoint/{}.pkn", mod, common::ToString(gpGlobals->mapname));

		std::ifstream ifs{ waypoint_path, std::ios_base::in | std::ios_base::binary};
		if (ifs.is_open()) {
			char header[20];
			ifs.read(header, sizeof(header));

			while (!ifs.eof()) {
				NodeID id{};
				ifs.read(reinterpret_cast<char*>(&id), sizeof(id));
				auto point = std::make_shared<Point>(Point{ {} });
				point->Read(&ifs);
				nodes.insert({ id, point });
				points_tree[PointAsIndex(point->Origin().z)][PointAsIndex(point->Origin().y)][PointAsIndex(point->Origin().x)].push_back(id);
			}
			return true;
		}
		return false;
	}
	
	bool Graph::Save() {
		char mod[50];
		g_engfuncs.pfnGetGameDir(mod);

		std::filesystem::path waypoint_path = std::format("{}/pokebot/data/waypoint/{}.pkn", mod, common::ToString(gpGlobals->mapname));
		std::filesystem::create_directories(waypoint_path.parent_path());

		std::ofstream ofs{ waypoint_path, std::ios_base::out | std::ios_base::binary};
		if (ofs.is_open()) {
			constexpr const char Header[20] = "POKEBOT_00000000000";
			ofs.write(Header, sizeof(Header));
			for (NodeID i = 0; i < nodes.size(); i++) {
				// Save nodes.
				if (auto it = nodes.find(i); it != nodes.end()) {
					ofs.write(reinterpret_cast<const char*>(&i), sizeof(i));
					std::static_pointer_cast<Point>(it->second)->Write(&ofs);
				}
			}
			return true;
		}
		return false;
	}

	Vector Graph::GetOrigin(const NodeID Node_ID) const noexcept {
		return (Node_ID != Invalid_NodeID ? nodes.at(Node_ID)->Origin() : Vector(9999, 9999, 9999));
	}

	std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> Point::GetConnections() const noexcept {
		return std::make_pair(connections.begin(), connections.end());
	}

	bool Point::AddConnection(const NodeID node_id) noexcept {
		connections.insert(node_id);
		return true;
	}

	bool Point::AddFlag(const NodeFlag flag) noexcept {
		this->flag |= flag;
		return true;
	}
}