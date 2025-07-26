module;
#include "common.hpp"
#include "goldsrc.hpp"
export module pokebot.game.player: action_key;

export namespace pokebot::game::player {
	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		ActionKey,
		None = 0,
		Run = IN_RUN,
		Attack = IN_ATTACK,
		Jump = IN_JUMP,
		Duck = IN_DUCK,
		Forward = IN_FORWARD,
		Back = IN_BACK,
		Use = IN_USE,
		Cancel = IN_CANCEL,
		Left = IN_LEFT,
		Right = IN_RIGHT,
		Move_Left = IN_MOVELEFT,
		Move_Right = IN_MOVERIGHT,
		Attack2 = IN_ATTACK2,
		Reload = IN_RELOAD,
		ALT1 = IN_ALT1,
		Score = IN_SCORE,
		Shift = 1 << 16
	);
}