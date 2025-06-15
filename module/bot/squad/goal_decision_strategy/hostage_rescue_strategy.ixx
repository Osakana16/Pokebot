export module pokebot.bot.squad.goal_decision_strategy:hostage_rescue;

import pokebot.bot.squad.strategy;
// import pokebot.game;

import :decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	export class RescueHostageStrategy : public DecisionStrategy {
		// pokebot::game::GameBase* game{};
	public:
		~RescueHostageStrategy() final {}
		RescueHostageStrategy(void*) {}

		//RescueHostageStrategy(pokebot::game::GameBase* game_) : game(game_) {}

		strategy::Objective Decide() final {
			return strategy::Objective{  };
		}
	};

	export class EscortHostageStrategy : public DecisionStrategy {
	public:
		~EscortHostageStrategy() final {}

		EscortHostageStrategy() {

		}

		strategy::Objective Decide() final {
			return strategy::Objective{  };
		}
	};
}