module pokebot.bot.behavior: action;

namespace pokebot::bot::behavior {
	Status Action::Evaluate(Bot* const self, game::CSGameBase* game, const node::Graph* const graph) noexcept {
		return action(self, game, graph);
	}

	void Action::Define(BehaviorFunc action_) noexcept {
		assert(action == nullptr);
		action = action_;
	}

	std::shared_ptr<Action> Action::Create(std::string_view name) {
		return std::make_shared<Action>(name);
	}

	std::shared_ptr<Action> Action::Create(BehaviorFunc action_, std::string_view name) {
		auto result = Create(name);
		result->Define(action_);
		return result;
	}
}