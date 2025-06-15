#include "navmesh/navigation_map.h"
export module pokebot.terrain.graph.node: node_flag;

export namespace pokebot::node {
	enum class NavmeshFlag {
		None = 0,
		Crouch = navmesh::NavAttributeType::NAV_CROUCH,
		Jump = navmesh::NavAttributeType::NAV_JUMP,
		No_Jump = navmesh::NavAttributeType::NAV_NO_JUMP
	};

	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		NodeFlag,
		None,
		Water = 1 << 1,
		Ladder = 1 << 2,
		Goal = 1 << 3
	);
}