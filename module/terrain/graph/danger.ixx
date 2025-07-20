export module pokebot.terrain.graph.danger;
import pokebot.terrain.graph.node;

export namespace pokebot::node {
	struct Danger final {
		std::unordered_map<NodeID, std::uint32_t> number_of_reference{};
	};
}