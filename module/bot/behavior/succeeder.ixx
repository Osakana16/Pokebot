export module pokebot.bot.behavior: succeeder;
import :behavior_node;
namespace pokebot::bot::behavior {
	class Succeeder : public BehaviorNode {
		std::shared_ptr<BehaviorNode> child{};
	public:
		Status Evaluate(Bot* const self, game::Game* game, const node::Graph* const graph) override {
			child->Evaluate(self, game, graph);
			return Status::Success;
		}

		Succeeder(std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("Succeeder"), child(behavior) {}
		static std::shared_ptr<Succeeder> As(std::shared_ptr<BehaviorNode> behavior) { return std::make_shared<Succeeder>(behavior); }
	};

	class Failder : public BehaviorNode {
		std::shared_ptr<BehaviorNode> child{};
	public:
		Status Evaluate(Bot* const self, game::Game* game, const node::Graph* const graph) override {
			child->Evaluate(self, game, graph);
			return Status::Failed;
		}

		Failder(std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("Succeeder"), child(behavior) {}
		static std::shared_ptr<Failder> As(std::shared_ptr<BehaviorNode> behavior) { return std::make_shared<Failder>(behavior); }
	};
}