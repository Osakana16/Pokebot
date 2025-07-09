export module pokebot.bot.behavior: action;
import :behavior_node;
namespace pokebot::bot::behavior {
	class Action : public BehaviorNode {
		BehaviorFunc action;
	public:
		using BehaviorNode::BehaviorNode;

		Status Evaluate(Bot* const self, game::Game*, const node::Graph* const graph) noexcept override;
		void Define(BehaviorFunc action_) noexcept;
		static std::shared_ptr<Action> Create(std::string_view name);
		static std::shared_ptr<Action> Create(BehaviorFunc action_, std::string_view name);
	};
}