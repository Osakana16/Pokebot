module;
#include "bit_operator_for_enum.hpp"

export module pokebot.game.player: item;
export namespace pokebot::game {
	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		Item,
		None,
		Nightvision = 1 << 0,
		Defuse_Kit = 1 << 1
	);
}