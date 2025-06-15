export module pokebot.bot.behavior: behavior_node;
import :behavior_node_status;

import pokebot.bot;

export namespace pokebot::bot::behavior {
	using BehaviorFunc = std::function<Status(Bot* const)>;
	using Activator = std::function<bool(const Bot* const)>;

	class BehaviorNode {
	protected:
		std::string_view name{};
	public:
		BehaviorNode(std::string_view self_name) : name(self_name) {}
		virtual Status Evaluate(Bot* const) = 0;
	};
}