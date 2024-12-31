#pragma once
#include "navmesh/navigation_map.h"
#define USE_NAVMESH true

namespace pokebot::node {
	using NodeID = std::int64_t;
	
	template<typename T>
	constexpr T World_Size = 8192;
	constexpr size_t Split_Size = 256;
	constexpr size_t Tree_Size = World_Size<size_t> / Split_Size;

	template<typename T>
	constexpr T Range = 50;
	constexpr NodeID Invalid_NodeID = std::numeric_limits<NodeID>::max();

	enum class NodeFlag {
		None,
		Water = 1 << 1,
		Ladder = 1 << 2,
		Goal = 1 << 3
	};
	POKEBOT_ENUM_BIT_OPERATORS(NodeFlag);

	enum class RouteState {
		New,
		Open,
		Closed
	};

	template<typename NodeIdentity>
	struct Route {
		float g{}, f{};
		std::conditional_t<std::is_same_v<NodeIdentity, navmesh::NavArea>, navmesh::NavArea*, NodeID> parent{};
		RouteState state = RouteState::New;

		constexpr Route() {
			if constexpr (std::is_same_v<NodeIdentity, NodeID>) {
				parent = Invalid_NodeID;
			} else if constexpr (std::is_same_v<NodeIdentity, navmesh::NavArea>) {
				parent = nullptr;
			}
		}
	};

	template<typename NodeIdentity>
	class PathWalk {
		std::list<NodeIdentity> nodes{};
		std::list<NodeIdentity>::const_iterator cursor{};
	public:
		void PushFront(const NodeIdentity n) noexcept {
			nodes.push_front(n);
			cursor = nodes.begin();
		}

		void PushBack(const NodeIdentity n) noexcept {
			nodes.push_back(n);
			cursor = nodes.begin();
		}

		NodeIdentity Current() const noexcept {
			return *cursor;
		}

		NodeIdentity Destination() const noexcept {
			return nodes.back();
		}

		bool Next() noexcept {
			cursor++;
			return !IsEnd();
		}

		void Previous() noexcept {
			cursor--;
		}

		bool IsEnd() const noexcept {
			return cursor == nodes.cend();
		}

		bool Empty() const noexcept {
			return nodes.empty();
		}

		void Clear() noexcept {
			nodes.clear();
		}
	};

	using RouteID = int;

	// Waypoint
	class Point final {
		inline operator Vector() const noexcept { return Origin(); }


		static constexpr float MAX_COST = std::numeric_limits<float>::max();
		std::unordered_set<NodeID> connections{};

		NodeFlag flag{};

		Vector point{};
	public:
		float time{};
		void Write(std::ofstream* const), Read(std::ifstream* const);
		const Vector& Origin() const noexcept;
		std::pair<float, float> Length() const noexcept;

		Point(const Vector& Initialize_Point) : point(Initialize_Point) {}

		std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> GetConnections() const noexcept;
		bool AddConnection(const NodeID) noexcept;
		bool AddFlag(const NodeFlag) noexcept;
	};

	enum class GoalKind {
		None,
		Terrorist_Spawn,
		CT_Spawn,
		Bombspot,
		Rescue_Zone,
		Escape_Zone,
		Vip_Safety,
		C4
	};
	constexpr auto All_Goal_List = { GoalKind::Terrorist_Spawn, GoalKind::CT_Spawn, GoalKind::Bombspot, GoalKind::Rescue_Zone, GoalKind::Escape_Zone, GoalKind::Vip_Safety };
#if !USE_NAVMESH
	inline class WaypointGraph {
		friend class FGreater;

		std::unordered_map<NodeID, std::shared_ptr<Point>> nodes{};
		std::unordered_multimap<GoalKind, NodeID> goals{};
		std::vector<NodeID> points_tree[Tree_Size][Tree_Size][Tree_Size]{};

		void AddBasic(); 
		std::shared_ptr<Point> GetNode(NodeID point_id);

		std::vector<Route<NodeID>> routes{};

		void Remove(NodeID point_id);
		bool Load();
		bool Save();
	public:
		void OnMapLoaded();
		void OnNewRound();
		NodeID Add(const Vector& Position, const GoalKind);
		void Remove(const Vector& Position);

		void Clear();

		NodeID TryToConnect(const NodeID Node_ID);

		bool IsSameGoal(const NodeID, const GoalKind) const noexcept;
		bool IsOnNode(const Vector& Position, const NodeID) const noexcept;
		Vector GetOrigin(const NodeID Node_ID) const noexcept;
		NodeID GetNearest(const Vector& Destination) const noexcept;

		void FindPath(PathWalk<NodeID>* const, const Vector&, const Vector&),
			FindPath(PathWalk<NodeID>* const, const NodeID, const NodeID);

		decltype(static_cast<const decltype(goals)>(goals).equal_range(GoalKind::None)) GetGoal(const GoalKind kind) const noexcept;

		bool IsPossibleToReachInTime() const noexcept;

	public:
		void Draw();
	} world;
#else
	enum class NavmeshFlag {
		None = 0,
		Crouch = navmesh::NavAttributeType::NAV_CROUCH,
		Jump = navmesh::NavAttributeType::NAV_JUMP,
		No_Jump = navmesh::NavAttributeType::NAV_NO_JUMP
	};

	struct Danger final {
		std::unordered_map<NodeID, std::uint32_t> number_of_reference{};
	};

	// Compatibility with ZBot navmesh.
	inline class CZBotGraph {
		std::unordered_multimap<GoalKind, NodeID> goals{};
		Danger danger[2]{};
	public:
		size_t GetNumberOfGoals(GoalKind) const noexcept;

		bool IsSameGoal(const NodeID, const GoalKind) const noexcept;
		bool IsOnNode(const Vector& Position, const NodeID) const noexcept;
		Vector GetOrigin(const NodeID Node_ID) const noexcept;

		// Call on map loaded.
		void OnMapLoaded();
		// Call on new round
		void OnNewRound();
		navmesh::NavigationMap navigation_map{};
		std::vector<Route<navmesh::NavArea*>> routes{};

		navmesh::NavArea* GetNearest(const Vector& Destination) const noexcept;
		
		decltype(static_cast<const decltype(goals)>(goals).equal_range(GoalKind::None)) GetGoal(const GoalKind kind) const noexcept;

		void FindPath(PathWalk<std::uint32_t>* const, const Vector&, const Vector&, const common::Team);
		
		bool HasFlag(const NodeID, NavmeshFlag) const noexcept;
	} czworld;
#endif
	void AStarSearch(std::pair<const Vector&, const Vector&>);
}