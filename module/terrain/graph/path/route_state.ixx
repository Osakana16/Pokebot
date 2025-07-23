export module pokebot.terrain.graph.path.route_state;

export namespace pokebot::node {
	enum class RouteState {
		New,
		Open,
		Closed
	};
}