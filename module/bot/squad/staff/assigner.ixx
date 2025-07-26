module;
#include <set>
#include <unordered_set>
export module pokebot.bot.squad.staff:assigner;
import pokebot.util;

namespace pokebot::bot::squad::staff {
	export class Assigner {
	protected:
		std::vector<std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash>>* const player_platoon;
		const std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash>* const Players;
	public:
		Assigner(decltype(player_platoon) player_platoon_, decltype(Players) Players_) : player_platoon(player_platoon_), Players(Players_) {}

		virtual void Assign(size_t max_number) = 0;
	};
}