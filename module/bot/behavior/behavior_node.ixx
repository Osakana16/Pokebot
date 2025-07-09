export module pokebot.bot.behavior: behavior_node;
import :behavior_node_status;

import pokebot.terrain.graph;
import pokebot.bot;
import pokebot.game;

export namespace pokebot::bot::behavior {
	using BehaviorFunc = std::function<Status(Bot* const, const game::Game* const, const node::Graph* const)>;
	using Activator = std::function<bool(const Bot* const, const game::Game* const, const node::Graph* const)>;

	class BehaviorNode {
	protected:
		std::string_view name{};
	public:
		BehaviorNode(std::string_view self_name) : name(self_name) {}
		virtual Status Evaluate(Bot* const, game::Game*, const node::Graph* const) = 0;
	};
}