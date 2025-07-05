export module pokebot.bot.behavior: priority;
import :behavior_node;

export namespace pokebot::bot::behavior {
	class Priority : public BehaviorNode {
		std::vector<std::shared_ptr<BehaviorNode>> children;
	public:
		using BehaviorNode::BehaviorNode;

		Status Evaluate(Bot* const self, const node::Graph* const graph) override {
			// SERVER_PRINT(std::format("{}\n", name).c_str());
			for (auto child : children) {
				switch (child->Evaluate(self, graph)) {
					case Status::Running:
						return Status::Running;
					case Status::Success:
						return Status::Success;
				}
			}
			return Status::Failed;
		}

		void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
			assert(children.empty());
			children = behaviors;
		}

		static std::shared_ptr<Priority> Create(const char* const Name) {
			auto node = std::make_shared<Priority>(Name);
			return node;
		}

		static std::shared_ptr<Priority> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
			auto result = Create("Priority");
			result->Define(behaviors);
			return result;
		}
	};
}