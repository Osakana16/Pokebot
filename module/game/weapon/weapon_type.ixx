export module pokebot.game.weapon: weapon_type;

export namespace pokebot::game::weapon {
	enum class WeaponType {
		Secondary,
		Primary,
		Melee,
		Grenade,
		Special
	};
}