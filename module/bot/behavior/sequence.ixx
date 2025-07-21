export module pokebot.bot.behavior: sequence;
import :behavior_node;

export namespace pokebot::bot::behavior {
	class Sequence : public BehaviorNode {
		std::vector<std::shared_ptr<BehaviorNode>> children;
	public:
		using BehaviorNode::BehaviorNode;

		Status Evaluate(Bot* const self, game::CSGameBase* game, const node::Graph* const graph) override {
			// SERVER_PRINT(std::format("{}\n", name).c_str());
			for (auto child : children) {
				switch (child->Evaluate(self, game, graph)) {
					case Status::Running:
						return Status::Running;
					case Status::Failed:
						return Status::Failed;
				}
			}
			return Status::Success;
		}

		void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
			assert(children.empty());
			children = behaviors;
		}

		static std::shared_ptr<Sequence> Create(const char* const Name) {
			auto node = std::make_shared<Sequence>(Name);
			return node;
		}

		static std::shared_ptr<Sequence> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
			auto result = Create("Sequence");
			result->Define(behaviors);
			return result;
		}
	};

}