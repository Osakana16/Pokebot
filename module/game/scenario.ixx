#include "bit_operator_for_enum.hpp"
export module pokebot.game.util: scenario;

export namespace pokebot::game {
	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		MapFlags,
		Demolition = 1 << 0,
		HostageRescue = 1 << 1,
		Assassination = 1 << 2,
		Escape = 1 << 3,
		Other = 1 << 4
	);
}