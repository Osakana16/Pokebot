#include "navmesh/navigation_map.h"
export module pokebot.terrain.graph.path: route;
import :route_state;
import pokebot.terrain.graph.node;
import std;

export namespace pokebot::node {
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
	
	using RouteID = int;
}