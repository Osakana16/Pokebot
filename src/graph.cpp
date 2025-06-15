#include "graph.hpp"
#include <unordered_set>
#include <fstream>

import pokebot.game;
import pokebot.game.util;

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

	const Vector& Point::Origin() const POKEBOT_NOEXCEPT {
		return point;
	}

	std::pair<float, float> Point::Length() const POKEBOT_NOEXCEPT {
		return std::make_pair(Range<float>, Range<float>);
	}

	size_t PointAsIndex(const float Pos) POKEBOT_NOEXCEPT {
		return static_cast<size_t>(Pos / Split_Size) + 16;
	}

#if !USE_NAVMESH
	NodeID WaypointGraph::TryToConnect(const NodeID Node_ID) {
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

	void WaypointGraph::Clear() {
		nodes.clear();
		for (int z = 0; z < Tree_Size; z++) {
			for (int y = 0; y < Tree_Size; y++) {
				for (int x = 0; x < Tree_Size; x++) {
					points_tree[z][y][x].clear();
				}
			}
		}
	}

	void WaypointGraph::Remove(NodeID point_id) {
		nodes.erase(point_id);
	}

	std::shared_ptr<Point> WaypointGraph::GetNode(NodeID point_id) {
		if (auto point_iterator = nodes.find(point_id); point_iterator != nodes.end())
			return point_iterator->second;
		else
			return nullptr;
	}

	NodeID WaypointGraph::Add(const Vector& Position, const GoalKind Kind) {
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
		
	bool WaypointGraph::IsSameGoal(const NodeID Node_ID, const GoalKind Goal_Kind) const POKEBOT_NOEXCEPT {
		auto goals = GetGoal(node::GoalKind::Bombspot);
		for (auto goal = goals.first; goal != goals.second; goal++) {
			if (Node_ID == goal->second) {
				return true;
			}
		}
		return false;
	}

	bool WaypointGraph::IsOnNode(const Vector& Position, const NodeID Target_Node_ID) const POKEBOT_NOEXCEPT {
		return (GetNearest(Position) == Target_Node_ID);
	}

	void WaypointGraph::Draw() {
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

	void WaypointGraph::AddBasic() {
		auto MoveOriginOnGround = [](Vector* const origin) POKEBOT_NOEXCEPT {
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
			NodeID point_id = Add(origin, GoalKind::Escape_Zone);
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

	void WaypointGraph::OnMapLoaded() {
		Clear();
	}

	void WaypointGraph::OnNewRound() {
		if (!Load()) {
			AddBasic();
		}
		Save();
	}
	
	void WaypointGraph::Remove(const Vector& Position) {
		if (auto point_id = GetNearest(Position); point_id != Invalid_NodeID) {
			Remove(point_id);
		}
	}

	void WaypointGraph::FindPath(PathWalk<NodeID>* const walk_routes, const Vector& Start, const Vector& Goal) {
		FindPath(walk_routes, GetNearest(Start), GetNearest(Goal));
	}


	void WaypointGraph::FindPath(PathWalk<NodeID>* const walk_routes, const NodeID start_node_id, const NodeID end_node_id) {
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
	}


	decltype(static_cast<const decltype(WaypointGraph::goals)>(WaypointGraph::goals).equal_range(GoalKind::None)) WaypointGraph::GetGoal(const GoalKind kind) const POKEBOT_NOEXCEPT {
		return goals.equal_range(kind);
	}

	NodeID WaypointGraph::GetNearest(const Vector& Destination) const POKEBOT_NOEXCEPT {
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

	bool WaypointGraph::Load() {
#if 0
		char mod[50];
		g_engfuncs.pfnGetGameDir(mod);

		std::filesystem::path waypoint_path = std::format("{}/pokebot/data/waypoint/{}.pkn", mod, common::ToString(gpGlobals->mapname));

		std::ifstream ifs{ waypoint_path, std::ios_base::in | std::ios_base::binary};
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
	
	bool WaypointGraph::Save() {
#if 0
		char mod[50];
		g_engfuncs.pfnGetGameDir(mod);

		std::filesystem::path waypoint_path = std::format("{}/pokebot/data/waypoint/{}.pkn", mod, common::ToString(gpGlobals->mapname));
		std::filesystem::create_directories(waypoint_path.parent_path());

		std::ofstream ofs{ waypoint_path, std::ios_base::out | std::ios_base::binary};
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

	Vector WaypointGraph::GetOrigin(const NodeID Node_ID) const POKEBOT_NOEXCEPT {
		return (Node_ID != Invalid_NodeID ? nodes.at(Node_ID)->Origin() : Vector(9999, 9999, 9999));
	}

	std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> Point::GetConnections() const POKEBOT_NOEXCEPT {
		return std::make_pair(connections.begin(), connections.end());
	}

	bool Point::AddConnection(const NodeID node_id) POKEBOT_NOEXCEPT {
		connections.insert(node_id);
		return true;
	}

	bool Point::AddFlag(const NodeFlag flag) POKEBOT_NOEXCEPT {
		this->flag |= flag;
		return true;
	}
#else
	navmesh::NavArea* CZBotGraph::GetNearest(const Vector& Destination, const float Beneath_Limit) const POKEBOT_NOEXCEPT {
		return navigation_map.GetNavArea(&Destination, Beneath_Limit);
	}

	void CZBotGraph::FindPath(PathWalk<std::uint32_t>* const walk_routes, const Vector& Source, const Vector& Destination, const game::Team Joined_Team) {
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
				return czworld.routes.at(a_id).f > czworld.routes.at(b_id).f;
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

	bool CZBotGraph::IsNavFileLoaded() const noexcept {
		return is_nav_loaded;
	}

	void CZBotGraph::OnMapLoaded() {
        if (!navigation_map.Load(std::format("cstrike/maps/{}.nav", STRING(gpGlobals->mapname)))) {
            if (!navigation_map.Load(std::format("czero/maps/{}.nav", STRING(gpGlobals->mapname)))) {
                SERVER_PRINT("[POKEBOT]Failed to load the nav file.\n");
				is_nav_loaded = false;
				return;
            } else {
                SERVER_PRINT("[POKEBOT]Loaded the nav file from czero.\n");
            }
        } else {        
            SERVER_PRINT("[POKEBOT]Loaded the nav file from cstrike.\n");
        }

		is_nav_loaded = true;
		auto addGoal = [this](const GoalKind kind, const char* class_name, Vector(*originFunction)(edict_t*)) {
			edict_t* entity = nullptr;
			while ((entity = game::FindEntityByClassname(entity, class_name)) != nullptr) {
				Vector origin = originFunction(entity);
				auto area = GetNearest(origin);
				if (area != nullptr) {
					goals.insert({ kind, area->m_id });
				}
			}
		};

		auto returnOrigin = [](edict_t* entity) { return entity->v.origin; };
		auto returnModelOrigin = [](edict_t* entity) { return game::VecBModelOrigin(entity); };

		goals.clear();
		addGoal(GoalKind::CT_Spawn, "info_player_start", returnOrigin);				// CT Spawn
		addGoal(GoalKind::Terrorist_Spawn, "info_player_deathmatch", returnOrigin);	// Terrorist Spawn
		addGoal(GoalKind::Rescue_Zone, "func_hostage_rescue", returnModelOrigin);
		addGoal(GoalKind::Rescue_Zone, "info_hostage_rescue", returnModelOrigin);
		addGoal(GoalKind::Bombspot, "func_bomb_target", returnModelOrigin);
		addGoal(GoalKind::Bombspot, "info_bomb_target", returnModelOrigin);
		addGoal(GoalKind::Vip_Safety, "info_vip_start", returnModelOrigin);
		addGoal(GoalKind::Vip_Safety, "func_vip_safetyzone", returnModelOrigin);
		addGoal(GoalKind::Escape_Zone, "func_escapezone", returnModelOrigin);
	}

	void CZBotGraph::OnNewRound() {

	}

	
	size_t CZBotGraph::GetNumberOfGoals(const GoalKind Kind) const noexcept {
		size_t number{};
		auto goals = GetNodeByKind(Kind);
		for (auto goal = goals.first; goal != goals.second; goal++) {
			number++;
		}
		return number;
	}

	bool CZBotGraph::IsSameGoal(const NodeID Node_ID, const GoalKind Goal_Kind) const POKEBOT_NOEXCEPT {
		auto goals = GetNodeByKind(Goal_Kind);
		for (auto goal = goals.first; goal != goals.second; goal++) {
			if (Node_ID == goal->second) {
				return true;
			}
		}
		return false;
	}

	bool CZBotGraph::IsOnNode(const Vector& Position, const NodeID Target_Node_ID) const POKEBOT_NOEXCEPT {
		auto area = GetNearest(Position);
		return (area != nullptr && area->m_id == Target_Node_ID);
	}

	Graph::GoalKindRange CZBotGraph::GetNodeByKind(const GoalKind kind) const POKEBOT_NOEXCEPT {
		return goals.equal_range(kind);
	}


	std::optional<HLVector> CZBotGraph::GetOrigin(const NodeID Node_ID) const POKEBOT_NOEXCEPT {
		if (auto area = navigation_map.GetNavAreaByID(Node_ID); area != nullptr) {
			return HLVector{ .x = area->m_center.x, .y = area->m_center.y, .z = area->m_center.z };
		} else {
			return std::nullopt;
		}
	}

	
	bool CZBotGraph::HasFlag(const NodeID id, NavmeshFlag flag) const POKEBOT_NOEXCEPT {
		auto area = navigation_map.GetNavAreaByID(id);
		return area != nullptr && bool(area->m_attributeFlags & static_cast<navmesh::NavAttributeType>(flag));
	}
#endif
}