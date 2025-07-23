export module pokebot.terrain.graph.node.id;

export namespace pokebot::node {
	using NodeID = std::int64_t;

	template<typename T>
	constexpr T Range = 50;
	constexpr NodeID Invalid_NodeID = std::numeric_limits<NodeID>::max();
}