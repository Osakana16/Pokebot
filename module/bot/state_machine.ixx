export module pokebot.bot: state_machine;

namespace pokebot::bot {
	enum class State {
		Accomplishment,	// The bot mainly does accomplish the mission
		Crisis,			// The bot is in dangerous situation.
		Follow,			// Follow the leader
		Stuck			// 
	};
}