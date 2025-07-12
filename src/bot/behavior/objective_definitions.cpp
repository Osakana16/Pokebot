module pokebot.bot.behavior: behavior_definitions;
import :action;
import :selector;
import :sequence;
import :condition;
import :succeeder;
import :behavior_declaration;

import std;
import pokebot.bot;
import pokebot.game;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.terrain.graph;
import pokebot.terrain.graph.node;
import pokebot.terrain.goal;
import pokebot.util;

namespace pokebot::bot::behavior {
	void DefineObjective() {
		reset_team_objective->Define
		({
			reset_goal,
			set_goal_team_objective
		 });

		demolition::t_plant->Define
		({
			Condition::If(
				IsTeamObjectiveSet<false>, 
				Sequence::Create({
					Succeeder::As(reset_team_objective),
					Succeeder::As(set_goal_team_objective)
				}),
				Selector::Create({
					find_goal,			// 
					head_to_goal,		// 
					change_c4,			// 
					demolition::plant	// 
				})
			)
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
			Selector::Create
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
			Selector::Create
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