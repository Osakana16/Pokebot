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

	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		NodeFlag,
		None,
		Water = 1 << 1,
		Ladder = 1 << 2,
		Goal = 1 << 3
	);

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
		size_t Size() const {
			return nodes.size();
		}

		void PushFront(const NodeIdentity n) POKEBOT_NOEXCEPT {
			nodes.push_front(n);
			cursor = nodes.begin();
		}

		void PushBack(const NodeIdentity n) POKEBOT_NOEXCEPT {
			nodes.push_back(n);
			cursor = nodes.begin();
		}

		NodeIdentity Current() const POKEBOT_NOEXCEPT {
			return *cursor;
		}

		NodeIdentity Destination() const POKEBOT_NOEXCEPT {
			return nodes.back();
		}

		bool Next() POKEBOT_NOEXCEPT {
			cursor++;
			return !IsEnd();
		}

		void Previous() POKEBOT_NOEXCEPT {
			cursor--;
		}

		bool IsEnd() const POKEBOT_NOEXCEPT {
			return cursor == nodes.cend();
		}

		bool Empty() const POKEBOT_NOEXCEPT {
			return nodes.empty();
		}

		void Clear() POKEBOT_NOEXCEPT {
			nodes.clear();
		}
	};

	using RouteID = int;

	// Waypoint
	class Point final {
		inline operator Vector() const POKEBOT_NOEXCEPT { return Origin(); }


		static constexpr float MAX_COST = std::numeric_limits<float>::max();
		std::unordered_set<NodeID> connections{};

		NodeFlag flag{};

		Vector point{};
	public:
		float time{};
		void Write(std::ofstream* const), Read(std::ifstream* const);
		const Vector& Origin() const POKEBOT_NOEXCEPT;
		std::pair<float, float> Length() const POKEBOT_NOEXCEPT;

		Point(const Vector& Initialize_Point) : point(Initialize_Point) {}

		std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> GetConnections() const POKEBOT_NOEXCEPT;
		bool AddConnection(const NodeID) POKEBOT_NOEXCEPT;
		bool AddFlag(const NodeFlag) POKEBOT_NOEXCEPT;
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

		bool IsSameGoal(const NodeID, const GoalKind) const POKEBOT_NOEXCEPT;
		bool IsOnNode(const Vector& Position, const NodeID) const POKEBOT_NOEXCEPT;
		Vector GetOrigin(const NodeID Node_ID) const POKEBOT_NOEXCEPT;
		NodeID GetNearest(const Vector& Destination) const POKEBOT_NOEXCEPT;

		void FindPath(PathWalk<NodeID>* const, const Vector&, const Vector&),
			FindPath(PathWalk<NodeID>* const, const NodeID, const NodeID);

		decltype(static_cast<const decltype(goals)>(goals).equal_range(GoalKind::None)) GetGoal(const GoalKind kind) const POKEBOT_NOEXCEPT;

		bool IsPossibleToReachInTime() const POKEBOT_NOEXCEPT;

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
		common::Array<Danger, 2> danger;
	public:
		size_t GetNumberOfGoals(GoalKind) const POKEBOT_NOEXCEPT;

		bool IsSameGoal(const NodeID, const GoalKind) const POKEBOT_NOEXCEPT;
		bool IsOnNode(const Vector& Position, const NodeID) const POKEBOT_NOEXCEPT;
		Vector GetOrigin(const NodeID Node_ID) const POKEBOT_NOEXCEPT;

		// Call on map loaded.
		void OnMapLoaded();
		// Call on new round
		void OnNewRound();
		navmesh::NavigationMap navigation_map{};
		std::vector<Route<navmesh::NavArea*>> routes{};

		navmesh::NavArea* GetNearest(const Vector& Destination) const POKEBOT_NOEXCEPT;
		
		decltype(static_cast<const decltype(goals)>(goals).equal_range(GoalKind::None)) GetGoal(const GoalKind kind) const POKEBOT_NOEXCEPT;

		void FindPath(PathWalk<std::uint32_t>* const, const Vector&, const Vector&, const common::Team);
		
		bool HasFlag(const NodeID, NavmeshFlag) const POKEBOT_NOEXCEPT;
	} czworld;
#endif
	void AStarSearch(std::pair<const Vector&, const Vector&>);
}