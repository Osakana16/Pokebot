#include "behavior.hpp"

namespace pokebot::bot::behavior {
	// - DEmolition Behaviors - 
	namespace demolition {
		std::shared_ptr<Priority> objective = Priority::Create();
		BEHAVIOR_PRIVATE {
			std::shared_ptr<Sequence> t_plant = Sequence::Create();
			std::shared_ptr<Sequence> t_defusing = Sequence::Create();
			std::shared_ptr<Sequence> ct_planted = Sequence::Create();
			std::shared_ptr<Sequence> ct_defusing = Sequence::Create();
			std::shared_ptr<Sequence> blow = Sequence::Create();
		}
	}

	// - Rescue Behaviors -
	namespace rescue {
		std::shared_ptr<Priority> objective = Priority::Create();
		BEHAVIOR_PRIVATE {
			std::shared_ptr<Sequence> ct_try = Sequence::Create();
			std::shared_ptr<Sequence> ct_leave = Sequence::Create();
			std::shared_ptr<Sequence> lead_hostage = Sequence::Create();
		}
	}

	// - ASsasination Behaviors -
	namespace assist {
		std::shared_ptr<Priority> objective = Priority::Create();
		BEHAVIOR_PRIVATE {
			std::shared_ptr<Sequence> ct_cover = Sequence::Create();
			std::shared_ptr<Sequence> ct_take_point = Sequence::Create();
			std::shared_ptr<Sequence> ct_vip_escape = Sequence::Create();
		}
	}
	
	// - EScape Behaviors -
	namespace escape {
		std::shared_ptr<Priority> objective = Priority::Create();
		BEHAVIOR_PRIVATE {
			std::shared_ptr<Sequence> t_get_primary = Sequence::Create();
			std::shared_ptr<Sequence> t_take_point = Sequence::Create();
		}
	}

	namespace coop {
		std::shared_ptr<Sequence> objective = Sequence::Create();
	}

	namespace elimination {
		std::shared_ptr<Priority> objective = Priority::Create();
	}
	std::shared_ptr<Sequence> t_ordinary = Sequence::Create();
	std::shared_ptr<Sequence> ct_ordinary = Sequence::Create();

	template<node::GoalKind kind>
	bool CanGo(const Bot* const Self) noexcept {
		return !IsOnGoal<true, kind>(Self);
	}

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

	void DefineObjective() {
		demolition::objective->Define
		({
			BEHAVIOR_IFELSE
			(
				Is<common::Team::T>,
				Priority::Create
				({
			 		BEHAVIOR_IFELSE_TEMPLATE
					(
						IsBombPlanted,
						t_ordinary,
						// Plant the bomb
						Priority::Create
						({
					 		BEHAVIOR_IFELSE_TEMPLATE
							(
								HasBomb,
								demolition::t_plant,
								t_ordinary
							)
						})
					)
				}),
				Is<common::Team::CT>,
				Priority::Create
				({
			 		BEHAVIOR_IFELSE_TEMPLATE
					(
						IsBombPlanted,
		 				Priority::Create
						({
							BEHAVIOR_IFELSE_TEMPLATE
							(
								IsOnBomb,
								demolition::ct_defusing,
								demolition::ct_planted
							)
						}),
						ct_ordinary
					)
				})
			)
		});

		assist::objective->Define
		({
			set_goal_vipsafety,
			head_and_discard_goal
		 });

		coop::objective->Define
		({
			follow_squad_leader,
			head_and_discard_goal
		});

		elimination::objective->Define
		({
			BEHAVIOR_IFELSE
			(
				Is<common::Team::T>,
				t_ordinary,
		 		Is<common::Team::CT>,
				ct_ordinary
			)
		});

		demolition::t_plant->Define
		({
			set_goal_bombspot,
			head_and_discard_goal,
			change_c4,
			fire
		});

		demolition::t_defusing->Define
		({

		});

		demolition::ct_defusing->Define
		({
			look_c4,
			use
		});

		demolition::ct_planted->Define
		({
			set_goal_c4,
			head_and_discard_goal
		});

		rescue::objective->Define
		({
			BEHAVIOR_IFELSE_TEMPLATE
			(
				IsEnoughToRescueHostage,
				rescue::ct_leave,
				rescue::ct_try
			)
		 });


		rescue::ct_leave->Define
		({
			set_goal_rescuezone,
			head_and_discard_goal
		 });

		rescue::ct_try->Define
		({
			set_goal_hostage,
			head_and_discard_goal,
			rescue::lead_hostage
		 });

		assist::ct_cover->Define
		({
			Condition::If(IsHelper<false>, join_vip_squad)
		 });

		assist::ct_take_point->Define
		({
			set_goal_vipsafety,
			head_to_goal
		 });

		assist::ct_vip_escape->Define
		({
			Condition::If
			(
				IsHelper<false>,
				Sequence::Create
				({
					create_vip_squad
				})
			),
			set_goal_vipsafety,
			head_and_discard_goal
		 });

		escape::objective->Define
		({
			BEHAVIOR_IFELSE_TEMPLATE
			(
				IsTimeEarly,
				escape::t_get_primary,
				escape::t_take_point
			)
		 });

		escape::t_get_primary->Define
		({
			set_goal_weapon,
			head_and_discard_goal
		 });

		escape::t_take_point->Define
		({
			set_goal_escapezone,
			head_and_discard_goal	 
		 });

		rescue::lead_hostage->Define
		({
			look_hostage,
			use
		});

		t_ordinary->Define
		({
			Condition::If(CanGo<node::GoalKind::Terrorist_Spawn>, set_goal_ctspawn),
			Condition::If(CanGo<node::GoalKind::CT_Spawn>, set_goal_tspawn),
			head_and_discard_goal
		});

		ct_ordinary->Define
		({
			Condition::If(CanGo<node::GoalKind::Terrorist_Spawn>, set_goal_tspawn),
			Condition::If(CanGo<node::GoalKind::CT_Spawn>, set_goal_ctspawn),
			head_and_discard_goal
		});
	}
}