export module pokebot.bot.client;

import pokebot.game;

export namespace pokebot::bot::client {
	class BotClient {
		game::CSGameBase* game_manager;
	public:
		BotClient() {}

	};
}