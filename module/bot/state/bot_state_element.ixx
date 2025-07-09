export module pokebot.bot.state: bot_state_element;
import :state_element;

import pokebot.bot;
import pokebot.game;

namespace pokebot::bot::state {
	class BotStateElement : public StateElement {
	protected:
		Bot* const self{};
		game::Game* const game{};
	public:
		BotStateElement(Bot* const target, game::Game* game_) noexcept : self(target), game(game_) {}

		virtual ~BotStateElement() override {}

	};
}