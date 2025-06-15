export module pokebot.bot: message;

export namespace pokebot::bot {
	enum class Message {
		Normal,
		Buy,
		Team_Select,
		Model_Select,
		Selection_Completed
	};
}