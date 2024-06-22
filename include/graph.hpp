#pragma once
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

	struct Route {
		float g{}, f{};
		NodeID parent = Invalid_NodeID;
		RouteState state = RouteState::New;
	};

	class PathWalk {
		std::list<node::NodeID> nodes{};
		std::list<node::NodeID>::const_iterator cursor{};
	public:
		void PushFront(const node::NodeID n) noexcept {
			nodes.push_front(n);
			cursor = nodes.begin();
		}

		void PushBack(const node::NodeID n) noexcept {
			nodes.push_back(n);
			cursor = nodes.begin();
		}

		node::NodeID Current() const noexcept {
			return *cursor;
		}

		NodeID Destination() const noexcept {
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

	class Node {
	public:
		virtual const Vector& Origin() const = 0;
		virtual std::pair<float, float> Length() const = 0;

		virtual std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> GetConnections() const = 0;

		virtual bool AddConnection(const NodeID) = 0;
		virtual bool AddFlag(const NodeFlag) = 0;
		inline operator Vector() const noexcept { return Origin(); }

		float time{};
	};

	// Waypoint
	class Point final : public Node {
		static constexpr float MAX_COST = std::numeric_limits<float>::max();
		std::unordered_set<NodeID> connections{};

		NodeFlag flag{};

		Vector point{};
	public:
		void Write(std::ofstream* const), Read(std::ifstream* const);
		const Vector& Origin() const noexcept final;
		std::pair<float, float> Length() const noexcept final;

		Point(const Vector& Initialize_Point) : point(Initialize_Point) {}

		std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> GetConnections() const noexcept final;
		bool AddConnection(const NodeID) noexcept final;
		bool AddFlag(const NodeFlag) noexcept final;
	};

	enum class GoalKind {
		None,
		Terrorist_Spawn,
		CT_Spawn,
		Bombspot,
		Rescue_Zone,
		Esacpe_Zone,
		Vip_Safety,
		C4
	};
	constexpr auto All_Goal_List = { GoalKind::Terrorist_Spawn, GoalKind::CT_Spawn, GoalKind::Bombspot, GoalKind::Rescue_Zone, GoalKind::Esacpe_Zone, GoalKind::Vip_Safety };

	inline class Graph {
		friend class FGreater;

		std::unordered_map<NodeID, std::shared_ptr<Node>> nodes{};
		std::unordered_multimap<GoalKind, NodeID> goals{};
		std::vector<NodeID> points_tree[Tree_Size][Tree_Size][Tree_Size]{};

		void AddBasic(); 
		std::shared_ptr<Node> GetNode(NodeID point_id);

		std::vector<Route> routes{};

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
		NodeID GetID(const Vector& Position) const noexcept;
		Vector GetOrigin(const NodeID Node_ID) const noexcept;
		NodeID GetNearest(const Vector& Destination) const noexcept;

		void FindPath(PathWalk* const, const Vector&, const Vector&),
			FindPath(PathWalk* const, const NodeID, const NodeID);

		decltype(static_cast<const decltype(goals)>(goals).equal_range(GoalKind::None)) GetGoal(const GoalKind kind) const noexcept;

		bool IsPossibleToReachInTime() const noexcept;

	public:
		void Draw();
	} world;

	void AStarSearch(std::pair<const Vector&, const Vector&>);
}