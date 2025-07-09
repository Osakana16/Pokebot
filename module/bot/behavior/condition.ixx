export module pokebot.bot.behavior: condition;
import :behavior_node;
namespace pokebot::bot::behavior {
	class Condition : public BehaviorNode {
		std::shared_ptr<BehaviorNode> child{};
		Activator CanActive;
	public:
		Status Evaluate(Bot* const self, game::Game* game, const node::Graph* const graph) override {
			if (CanActive(self, game, graph)) {
				return child->Evaluate(self, game, graph);
			}
			return Status::Not_Executed;
		}

		Condition(Activator activator, std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("If"), CanActive(activator), child(behavior) {}

		static std::shared_ptr<Condition> If(Activator activator, std::shared_ptr<BehaviorNode> behavior) { return std::make_shared<Condition>(activator, behavior); }
	};
}