#include "behavior.hpp"

namespace pokebot::bot::behavior {
	// - Rescue Behaviors -
	namespace rescue {
		std::shared_ptr<Sequence> ct_try = Sequence::Create("rescue::ct_try");
		std::shared_ptr<Sequence> ct_leave = Sequence::Create("rescue::ct_leave");
		std::shared_ptr<Sequence> lead_hostage = Sequence::Create("rescue::lead_hostage");
	}

	// - ASsasination Behaviors -
	namespace assist {
		std::shared_ptr<Sequence> ct_cover = Sequence::Create("assist::ct_cover");
		std::shared_ptr<Sequence> ct_take_point = Sequence::Create("assist::ct_take_point");
		std::shared_ptr<Sequence> ct_vip_escape = Sequence::Create("assist::ct_vip_escape");
	}
	
	// - EScape Behaviors -
	namespace escape {
		std::shared_ptr<Sequence> t_get_primary = Sequence::Create("escape::t_get_primary");
		std::shared_ptr<Sequence> t_take_point = Sequence::Create("escape::t_take_point");
	}

	std::shared_ptr<Sequence> t_ordinary = Sequence::Create("elimination::t_ordinary");
	std::shared_ptr<Sequence> ct_ordinary = Sequence::Create("elimination::ct_ordinary");

	BEHAVIOR_CREATE(Sequence, reset_team_objective);

	template<bool b>
	bool IsEnoughToRescueHostage(const Bot* const Self) noexcept {
		// Return true if the following conditions meet the requirements
		// 1. I have some hostages.
		// 2. Teammates are leading some hostages.
		if constexpr (b) {
			return false;
		} else {
			return true;
		}
	}
	
	bool CanUseHostage(const Bot* const Self) noexcept {
		return game::game.GetClosedHostage(Self->Origin(), 83.0f) != nullptr;
	}

	template<bool b>
	bool IsFarFromC4(const Bot* const Self) noexcept {
		assert(bot::manager.C4Origin().has_value());
		if constexpr (b) {
			return common::Distance(Self->Origin(), *bot::manager.C4Origin()) > 100.0f;
		} else {
			return common::Distance(Self->Origin(), *bot::manager.C4Origin()) <= 100.0f;
		}
	}

	template<bool b>
	bool IsFarFromMainGoal(const Bot* const Self) noexcept {
		auto id = manager.GetGoalNode(Self->JoinedTeam(), Self->JoinedPlatoon());
		auto origin = node::czworld.GetOrigin(id);
		if constexpr (b) {
			return common::Distance(Self->Origin(), origin) > 100.0f;
		} else {
			return common::Distance(Self->Origin(), origin) <= 100.0f;
		}
	}

	void DefineObjective() {
		reset_team_objective->Define
		({
			reset_goal,
			set_goal_team_objective
		 });

		demolition::t_plant->Define
		({
			Condition::If(IsTeamObjectiveSet<false>, reset_team_objective),
			find_goal,
			head_to_goal,
			change_c4,
			rapid_fire
		});

		demolition::t_planted_wary->Define
		({
			Condition::If(IsFarFromC4<true>, set_goal_c4_node),
			Condition::If(IsFarFromC4<false>, set_goal_from_c4_within_range),
			find_goal,
			head_to_goal
		});

		demolition::t_planted_camp->Define
		({

		});

		demolition::t_defusing->Define
		({

		});

		demolition::ct_defend->Define
		({
			demolition::ct_defend_wary
		});
		
		demolition::ct_defend_wary->Define
		({
			Condition::If(IsFarFromMainGoal<true>, set_goal_team_objective),
			Condition::If(IsFarFromMainGoal<false>, set_goal_from_team_objective_within_range),
			find_goal,
			head_to_goal
		});

		demolition::ct_defend_camp->Define
		({

		});

		demolition::ct_defusing->Define
		({
			look_c4,
			use
		});

		demolition::ct_planted->Define
		({
			set_goal_c4_node,
			find_goal,
			head_to_goal,
			set_goal_c4_vector,
			move_vector
		});

		rescue::ct_leave->Define
		({
			set_goal_rescuezone,
			find_goal,
			head_to_goal
		 });

		rescue::ct_try->Define
		({
			set_goal_hostage,
			find_goal,
			head_to_goal,
			Condition::If(CanUseHostage, rescue::lead_hostage)
		 });

		assist::ct_cover->Define
		({

		 });

		assist::ct_take_point->Define
		({
			set_goal_vipsafety,
			find_goal,
			head_to_goal
		 });

		assist::ct_vip_escape->Define
		({
			set_goal_vipsafety,
			find_goal,
			head_to_goal
		 });

		escape::t_get_primary->Define
		({
			set_goal_weapon,
			find_goal,
			head_to_goal
		 });

		escape::t_take_point->Define
		({
			set_goal_escapezone,
			find_goal,
			head_to_goal	 
		 });

		rescue::lead_hostage->Define
		({
			look_hostage,
			use
		});

		t_ordinary->Define
		({
			Priority::Create
			(
				{
					set_goal_tspawn,
					set_goal_ctspawn
				}
			),
			head_to_goal
		});

		ct_ordinary->Define
		({
			Priority::Create
			(
				{
					set_goal_tspawn,
					set_goal_ctspawn
				}
			),
			head_to_goal
		});
	}
}