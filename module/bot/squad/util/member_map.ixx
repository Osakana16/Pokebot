module;
#include "util/fixed_string.hpp"
#include <unordered_map>

export module pokebot.bot.squad.util:member_map;

namespace pokebot::bot::squad::util {
	export using MemberMap = std::unordered_map<pokebot::util::PlayerName, int, pokebot::util::PlayerName::Hash>;
	export using MemberSet = std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash>;
}