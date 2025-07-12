export module pokebot.bot.behavior: behavior_declaration;
import :sequence;
import :selector;
import :action;

#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const self, game::Game* game, const node::Graph* const graph) POKEBOT_NOEXCEPT { return A; }
#define BEHAVIOR_IFELSE(AIF,A_PROCESS,BIF,B_PROCESS) Condition::If(AIF, A_PROCESS), Condition::If(BIF, B_PROCESS)
#define BEHAVIOR_IFELSE_TEMPLATE(IF,A_PROCESS,B_PROCESS) Condition::If(IF<true>, A_PROCESS), Condition::If(IF<false>, B_PROCESS)

#define RETURN_BEHAVIOR_TRUE_OR_FALSE(B,PROCESS) if constexpr (B) return (PROCESS); else return !(PROCESS)

#define BEHAVIOR_CREATE(TYPE,NAME) std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)

export namespace pokebot::bot::behavior {
	BEHAVIOR_CREATE(Action, change_primary);
	BEHAVIOR_CREATE(Action, change_secondary);
	BEHAVIOR_CREATE(Action, change_melee);
	BEHAVIOR_CREATE(Action, change_grenade);
	BEHAVIOR_CREATE(Action, change_flashbang);
	BEHAVIOR_CREATE(Action, change_smoke);
	BEHAVIOR_CREATE(Action, change_c4);
	BEHAVIOR_CREATE(Action, look_c4);
	BEHAVIOR_CREATE(Action, look_hostage);
	BEHAVIOR_CREATE(Action, look_enemy);
	BEHAVIOR_CREATE(Action, look_door);
	BEHAVIOR_CREATE(Action, look_button);
	BEHAVIOR_CREATE(Action, move_forward);
	BEHAVIOR_CREATE(Action, use);
	BEHAVIOR_CREATE(Action, tap_fire);
	BEHAVIOR_CREATE(Action, jump);
	BEHAVIOR_CREATE(Action, duck);
	BEHAVIOR_CREATE(Action, walk);
	BEHAVIOR_CREATE(Action, change_silencer);
	BEHAVIOR_CREATE(Action, adjust_scope);
	BEHAVIOR_CREATE(Action, set_goal_hostage_node);
	BEHAVIOR_CREATE(Action, set_goal_hostage_vector);
	BEHAVIOR_CREATE(Action, set_goal_bombspot);
	BEHAVIOR_CREATE(Action, set_goal_rescuezone);
	BEHAVIOR_CREATE(Action, set_goal_escapezone);
	BEHAVIOR_CREATE(Action, set_goal_vipsafety);
	BEHAVIOR_CREATE(Action, set_goal_tspawn);
	BEHAVIOR_CREATE(Action, set_goal_ctspawn);
	BEHAVIOR_CREATE(Action, set_goal_weapon);
	BEHAVIOR_CREATE(Action, find_goal);
	BEHAVIOR_CREATE(Action, head_to_goal);
	BEHAVIOR_CREATE(Action, set_goal_team_objective);
	BEHAVIOR_CREATE(Action, set_goal_from_c4_within_range);
	BEHAVIOR_CREATE(Action, reset_goal);
	BEHAVIOR_CREATE(Action, set_goal_c4_node);
	BEHAVIOR_CREATE(Action, set_goal_c4_vector);
	BEHAVIOR_CREATE(Action, set_goal_backpack_node);
	BEHAVIOR_CREATE(Action, set_goal_backpack_vector);
	BEHAVIOR_CREATE(Action, rapid_fire);
	BEHAVIOR_CREATE(Action, lock);
	BEHAVIOR_CREATE(Action, move_vector);
	BEHAVIOR_CREATE(Action, set_goal_from_team_objective_within_range);

	namespace fight {
		BEHAVIOR_CREATE(Sequence, beat_enemies);
		BEHAVIOR_CREATE(Selector, retreat);
	}

	// - DEmolition Behaviors - 
	namespace demolition {
		/*
			The bomber plants the bomb.
		*/
		BEHAVIOR_CREATE(Sequence, t_plant);

		BEHAVIOR_CREATE(Selector, t_planted_wary);	// Terrorists make the rounds to defend the bomb.
		BEHAVIOR_CREATE(Selector, t_planted_camp);	// Terrorirts camp around c4 to defend the bomb.
		BEHAVIOR_CREATE(Selector, t_pick_up_bomb);	// Terrorirts try to pick up the bomb.

		BEHAVIOR_CREATE(Selector, t_defusing);

		BEHAVIOR_CREATE(Selector, ct_defend);
		BEHAVIOR_CREATE(Selector, ct_defend_wary);
		BEHAVIOR_CREATE(Selector, ct_defend_camp);
		BEHAVIOR_CREATE(Selector, ct_planted);
		BEHAVIOR_CREATE(Sequence, ct_defusing);
	}

	// - Rescue Behaviors -
	namespace rescue {
		BEHAVIOR_CREATE(Selector, t_defend_hostage);

		BEHAVIOR_CREATE(Selector, ct_try);
		BEHAVIOR_CREATE(Selector, ct_leave);
		BEHAVIOR_CREATE(Sequence, lead_hostage);
	}

	// - ASsassination Behaviors -
	namespace assassination {
		extern std::shared_ptr<Sequence> ct_cover;
		extern std::shared_ptr<Sequence> ct_take_point;
		extern std::shared_ptr<Sequence> ct_vip_escape;
	}

	// - EScape Behaviors -
	namespace escape {
		extern std::shared_ptr<Sequence> t_get_primary;
		extern std::shared_ptr<Sequence> t_take_point;
	}

	std::shared_ptr<Action> wait(std::uint32_t, float);
}