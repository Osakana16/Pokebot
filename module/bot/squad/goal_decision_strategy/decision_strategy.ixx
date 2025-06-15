import pokebot.bot.squad.strategy;
export module pokebot.bot.squad.goal_decision_strategy:decision_strategy;

namespace pokebot::bot::squad::goal_decision_strategy {
	export class DecisionStrategy {
	public:
		virtual ~DecisionStrategy() = 0 {}
		virtual strategy::Objective Decide() = 0;
	};
}