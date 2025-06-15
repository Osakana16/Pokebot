export module pokebot.bot: personality_item;

namespace pokebot::bot {
	template<int min, int max>
	class PersonalityItem {
		int value{};
	public:
		inline PersonalityItem& operator=(const auto& v) POKEBOT_NOEXCEPT { value = std::clamp(v, min, max); return *this; }
		operator int() const POKEBOT_NOEXCEPT { return value; }
		PersonalityItem() : PersonalityItem(0) {}
		PersonalityItem(const auto& v) { operator=(v); }
	};
}