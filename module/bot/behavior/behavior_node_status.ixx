export module pokebot.bot.behavior: behavior_node_status;

namespace pokebot::bot::behavior {
	export enum class Status {
		Failed,
		Success,
		Running,
		Not_Executed
	};
}