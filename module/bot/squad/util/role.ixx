module;
#include <algorithm>

export module pokebot.bot.squad.util:role;
import pokebot.game.util;

namespace pokebot::bot::squad::util {
	export enum class Role {
		Offender,
		Defender,
		Both,
		Neither
	};

	export Role JudgeRole(game::Team base_team, game::MapFlags base_map_flags) noexcept {
		assert((base_team == game::Team::T || base_team == game::Team::CT) && "You assigned spector or random.");

		Role role = Role::Neither;
		const bool Is_Offender_True[2][2]{
			// Terrorist
			{
				bool(base_map_flags & game::MapFlags::Demolition),
				bool(base_map_flags & game::MapFlags::Escape)
			},
			// Counter-Terrorist
			{
				bool(base_map_flags & game::MapFlags::HostageRescue),
				bool(base_map_flags & game::MapFlags::Assassination)
			}
		};


		const bool Is_Defender_True[2][2]{
			// Terrorist
			{
				bool(base_map_flags & game::MapFlags::HostageRescue),
				bool(base_map_flags & game::MapFlags::Assassination)
			},
			// Counter-Terrorist
			{
				bool(base_map_flags & game::MapFlags::Demolition),
				bool(base_map_flags & game::MapFlags::Escape)
			}
		};

		auto isTrue = [](bool b) { return b; };
		const bool Is_Offender = std::any_of(std::begin(Is_Offender_True[static_cast<int>(base_team) - 1]), std::end(Is_Offender_True[static_cast<int>(base_team) - 1]), isTrue);
		const bool Is_Defender = std::any_of(std::begin(Is_Defender_True[static_cast<int>(base_team) - 1]), std::end(Is_Defender_True[static_cast<int>(base_team) - 1]), isTrue);

		if (Is_Offender && Is_Defender) {
			role = Role::Both;
		} else if (Is_Offender) {
			role = Role::Offender;
		} else if (Is_Defender) {
			role = Role::Defender;
		} else {
			role = Role::Neither;
		}
		return role;
	}
}