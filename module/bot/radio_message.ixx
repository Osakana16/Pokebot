export module pokebot.bot.radio_message;
import pokebot.game.util;
import pokebot.util;

export namespace pokebot::bot {
	struct RadioMessage {
		game::Team team = game::Team::Spector;
		pokebot::util::PlayerName sender{};
		util::fixed_string<255u> message{};
	};
}