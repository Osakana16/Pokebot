module;
#include "bit_operator_for_enum.hpp"

export module pokebot.game.player: status_icon;

export namespace pokebot::game {
	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		StatusIcon,
		Not_Displayed,
		Buy_Zone = 1 << 0,
		Defuser = 1 << 1,
		C4 = 1 << 2,
		Rescue_Zone = 1 << 3,
		Vip_Safety = 1 << 4,
		Escape_Zone = 1 << 5
	);
}