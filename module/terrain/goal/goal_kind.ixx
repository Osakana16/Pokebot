export module pokebot.terrain.goal:goal_kind;

export namespace pokebot::node {
	enum class GoalKind {
		None,
		Terrorist_Spawn,
		CT_Spawn,
		Bombspot,
		Rescue_Zone,
		Escape_Zone,
		Vip_Safety,
		C4
	};
}