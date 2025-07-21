export module pokebot.bot.behavior.node: condition;
import :behavior_node;

export namespace pokebot::bot::behavior {
	class Condition : public BehaviorNode {
		std::shared_ptr<BehaviorNode> true_child{};
		std::shared_ptr<BehaviorNode> false_child{};
		Activator CanActive;
	public:
		Status Evaluate(Bot* const self, game::CSGameBase* game, const node::Graph* const graph) override {
			if (CanActive(self, game, graph)) {
				return true_child->Evaluate(self, game, graph);
			} else {
				if (false_child) {
					return false_child->Evaluate(self, game, graph);
				}
			}
			return Status::Not_Executed;
		}

		Condition(Activator activator, std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("If"), CanActive(activator), true_child(behavior) {}
		Condition(Activator activator, std::shared_ptr<BehaviorNode> true_behavior, std::shared_ptr<BehaviorNode> false_behavior) : BehaviorNode("If"), CanActive(activator), true_child(true_behavior), false_child(false_behavior) {}

		static std::shared_ptr<Condition> If(Activator activator, std::shared_ptr<BehaviorNode> behavior) { return std::make_shared<Condition>(activator, behavior); }
		static std::shared_ptr<Condition> If(Activator activator, std::shared_ptr<BehaviorNode> true_behavior, std::shared_ptr<BehaviorNode> false_behavior) { return std::make_shared<Condition>(activator, true_behavior, false_behavior); }
	};
}