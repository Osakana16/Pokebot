export module pokebot.bot: mood;
import :personality_item;

namespace pokebot::bot {
	struct Mood {
		PersonalityItem<0, 100> brave{};
		PersonalityItem<0, 100> coop{};
	};
}