export module pokebot.bot.behavior: action;
import :behavior_node;
namespace pokebot::bot::behavior {
	class Action : public BehaviorNode {
		BehaviorFunc action;
	public:
		using BehaviorNode::BehaviorNode;

		Status Evaluate(Bot* const self) POKEBOT_NOEXCEPT override {
			// SERVER_PRINT(std::format("{}\n", name).c_str());
			return action(self);
		}

		void Define(BehaviorFunc action_) POKEBOT_NOEXCEPT {
			assert(action == nullptr);
			action = action_;
		}

		static std::shared_ptr<Action> Create(std::string_view name) {
			return std::make_shared<Action>(name);
		}

		static std::shared_ptr<Action> Create(BehaviorFunc action_, std::string_view name) {
			auto result = Create(name);
			result->Define(action_);
			return result;
		}
	};
}