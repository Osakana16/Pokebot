#include "behavior.hpp"
#include "bot/manager.hpp"

import pokebot.game.util;
namespace pokebot::bot::behavior {
	namespace demolition {
		std::shared_ptr<Sequence> plant = Sequence::Create("demolition::plant");
		std::shared_ptr<Sequence> defuse = Sequence::Create("demolition::defuse");
	}

	// - Rescue Behaviors -
	namespace rescue {
		std::shared_ptr<Sequence> head_to_hostage = Sequence::Create("head_to_hostage");
		auto get_closer_to_hostage = Priority::Create("use_to_rescue");
		auto use_to_rescue = Sequence::Create("use_to_rescue");
	}

	// - ASsasination Behaviors -
	namespace assassination {
		std::shared_ptr<Sequence> ct_cover = Sequence::Create("assassination::ct_cover");
		std::shared_ptr<Sequence> ct_take_point = Sequence::Create("assassination::ct_take_point");
		std::shared_ptr<Sequence> ct_vip_escape = Sequence::Create("assassination::ct_vip_escape");
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
	bool IsEnoughToRescueHostage(const Bot* const Self) POKEBOT_NOEXCEPT {
		// Return true if the following conditions meet the requirements
		// 1. I have some hostages.
		// 2. Teammates are leading some hostages.
		if constexpr (b) {
			return false;
		} else {
			return true;
		}
	}
	
	template<bool b>
	bool IsTeamObjectiveSet(const Bot* const Self) POKEBOT_NOEXCEPT {
		if constexpr (b) {
			return Self->goal_node == Manager::Instance().GetGoalNode(Self->Name().c_str());
		} else {
			auto troops_goal_node = Manager::Instance().GetGoalNode(Self->Name().c_str());
			return Self->goal_node != troops_goal_node;
		}
	}

	bool CanUseHostage(const Bot* const Self) POKEBOT_NOEXCEPT {
		return game::game.GetClosedHostage(Self->Origin(), 83.0f) != nullptr;
	}

	template<bool b>
	bool IsFarFromC4(const Bot* const Self) POKEBOT_NOEXCEPT {
		assert(bot::Manager::Instance().C4Origin().has_value());
		if constexpr (b) {
			return game::Distance(Self->Origin(), *bot::Manager::Instance().C4Origin()) > 100.0f;
		} else {
			return game::Distance(Self->Origin(), *bot::Manager::Instance().C4Origin()) <= 100.0f;
		}
	}

	template<bool b>
	bool IsFarFromMainGoal(const Bot* const Self) POKEBOT_NOEXCEPT {
		auto id = Manager::Instance().GetGoalNode(Self->Name().c_str());
		auto origin = node::czworld.GetOrigin(id);
		auto source = Self->Origin();
		if constexpr (b) {
			return game::Distance(source, *reinterpret_cast<Vector*>(&origin)) > 200.0f;
		} else {
			return game::Distance(source, *reinterpret_cast<Vector*>(&origin)) <= 200.0f;
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
			demolition::plant
		});

		demolition::plant->Define
		({
			lock,
			rapid_fire
		 });
		
		demolition::t_pick_up_bomb->Define
		({
			set_goal_backpack_node,
			find_goal,
			head_to_goal,
			set_goal_backpack_vector,
			move_vector
		});

		demolition::defuse->Define
		({
			lock,
			use
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
			demolition::defuse
		});

		demolition::ct_planted->Define
		({
			set_goal_c4_node,
			find_goal,
			head_to_goal,
			set_goal_c4_vector,
			move_vector
		 });

		/*
			Hostage Rescue
		*/

		rescue::t_defend_hostage->Define
		({
			Condition::If(IsFarFromMainGoal<true>, set_goal_team_objective),
			Condition::If(IsFarFromMainGoal<false>, set_goal_from_team_objective_within_range),
			find_goal,
			head_to_goal,
		 });

		rescue::ct_leave->Define
		({
			set_goal_rescuezone,
			find_goal,
			head_to_goal
		 });

		/* 
			CT: Try to rescue a hostage.


		*/
		rescue::ct_try->Define
		({
			set_goal_hostage_node,
			find_goal,
			head_to_goal,
			rescue::use_to_rescue
		 });

		rescue::use_to_rescue->Define
		({
			rescue::get_closer_to_hostage,
			Condition::If(CanUseHostage, rescue::lead_hostage)
		 });

		rescue::get_closer_to_hostage->Define
		({
			set_goal_hostage_vector,
			move_vector
		 });
		
		rescue::lead_hostage->Define
		({
			look_hostage,
			wait(1, .0f),
			use
		});

		assassination::ct_cover->Define
		({

		 });

		assassination::ct_take_point->Define
		({
			set_goal_vipsafety,
			find_goal,
			head_to_goal
		 });

		assassination::ct_vip_escape->Define
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