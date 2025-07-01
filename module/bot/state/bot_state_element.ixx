export module pokebot.bot.state: bot_state_element;
import :state_element;

import pokebot.bot;

namespace pokebot::bot::state {
	class BotStateElement : public StateElement {
	protected:
		Bot* const self{};
	public:
		BotStateElement(Bot* const target) noexcept : self(target) {}

		virtual ~BotStateElement() override {}

	};
}