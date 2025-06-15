#pragma once

namespace pokebot::node {
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

	struct HLVector {
		float x, y, z;
	};
		
	using NodeID = std::int64_t;
	constexpr auto All_Goal_List = { GoalKind::Terrorist_Spawn, GoalKind::CT_Spawn, GoalKind::Bombspot, GoalKind::Rescue_Zone, GoalKind::Escape_Zone, GoalKind::Vip_Safety };
	class Graph {
	public:
		using GoalKindRange = decltype(static_cast<const std::unordered_multimap<pokebot::node::GoalKind, NodeID>>(std::unordered_multimap<pokebot::node::GoalKind, NodeID>()).equal_range(pokebot::node::GoalKind::None));

		virtual GoalKindRange GetNodeByKind(const pokebot::node::GoalKind kind) const = 0;
		virtual std::optional<HLVector> GetOrigin(const NodeID Node_ID) const = 0;
		virtual size_t GetNumberOfGoals(GoalKind) const = 0;
	};
}