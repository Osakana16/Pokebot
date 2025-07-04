module;

import pokebot.game.util;
export module pokebot.bot.squad.staff:squad_staff;

namespace pokebot::bot::squad::staff {
	export class SquadStaff {
	protected:
		game::Team base_team{};
		game::MapFlags base_map_flags{};
	public:
		SquadStaff(game::Team team, game::MapFlags flag) : base_team(team), base_map_flags(flag) {}
	};
}