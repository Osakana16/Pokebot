import pokebot.bot.squad.strategy;

export module pokebot.bot.squad.goal_decision_strategy:prevent_hostage_rescue;
import :decision_strategy;


namespace pokebot::bot::squad::goal_decision_strategy {
	export class DefendHostageStrategy : public DecisionStrategy {
	public:
		~DefendHostageStrategy() final {}

		DefendHostageStrategy() {

		}

		strategy::Objective Decide() final {
			return strategy::Objective{  };
		}
	};

	export class DefendRescueZone : public DecisionStrategy {
	public:
		~DefendRescueZone() final {}

		DefendRescueZone() {}

		strategy::Objective Decide() final {
			return strategy::Objective{  };
		}
	};
}