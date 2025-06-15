export module pokebot.bot.squad.strategy:strategy_element;

export namespace pokebot::bot::squad::strategy {
	using LeaderName = pokebot::util::PlayerName;
	using GoalNode = node::NodeID;
	using HostageIDs = int;

	using Objective = std::variant<GoalNode, LeaderName, HostageIDs>;
}