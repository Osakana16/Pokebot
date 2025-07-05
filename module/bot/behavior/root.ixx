export module pokebot.bot.behavior: root;
import :behavior_node;

export namespace pokebot::bot::behavior {
	class Root : public BehaviorNode {
		std::shared_ptr<BehaviorNode> child{};
		Status Evaluate(Bot* const self, const node::Graph* const graph) override {
			return child->Evaluate(self, graph);
		}
	public:
		Root(std::shared_ptr<BehaviorNode> child_) : BehaviorNode("Root"), child(child_) {}
	};
}