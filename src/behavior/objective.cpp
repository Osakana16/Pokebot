#include "behavior.hpp"

namespace pokebot::bot::behavior {
	// - DEmolition Behaviors - 
	namespace demolition {
		std::shared_ptr<Priority> t_plant = Priority::Create("demolition::t_plant", Status::Executed);
		std::shared_ptr<Priority> t_defusing = Priority::Create("demolition::t_defusing", Status::Executed);
		std::shared_ptr<Priority> ct_planted = Priority::Create("demolition::ct_planted", Status::Executed);
		std::shared_ptr<Priority> ct_defusing = Priority::Create("demolition::ct_defusing", Status::Executed);
		std::shared_ptr<Priority> blow = Priority::Create("demolition::blow", Status::Executed);
	}

	// - Rescue Behaviors -
	namespace rescue {
		std::shared_ptr<Sequence> ct_try = Sequence::Create("rescue::ct_try", Status::Executed);
		std::shared_ptr<Sequence> ct_leave = Sequence::Create("rescue::ct_leave", Status::Executed);
		std::shared_ptr<Sequence> lead_hostage = Sequence::Create("rescue::lead_hostage", Status::Executed);
	}

	// - ASsasination Behaviors -
	namespace assist {
		std::shared_ptr<Sequence> ct_cover = Sequence::Create("assist::ct_cover", Status::Executed);
		std::shared_ptr<Sequence> ct_take_point = Sequence::Create("assist::ct_take_point", Status::Executed);
		std::shared_ptr<Sequence> ct_vip_escape = Sequence::Create("assist::ct_vip_escape", Status::Executed);
	}
	
	// - EScape Behaviors -
	namespace escape {
		std::shared_ptr<Sequence> t_get_primary = Sequence::Create("escape::t_get_primary", Status::Executed);
		std::shared_ptr<Sequence> t_take_point = Sequence::Create("escape::t_take_point", Status::Executed);
	}

	std::shared_ptr<Sequence> t_ordinary = Sequence::Create("elimination::t_ordinary", Status::Executed);
	std::shared_ptr<Sequence> ct_ordinary = Sequence::Create("elimination::ct_ordinary", Status::Executed);

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

	void DefineObjective() {
		demolition::t_plant->Define
		({
			set_goal_bombspot,
			After<Status::Enough>::With(
				head_and_discard_goal, Priority::Create(
				{
					change_c4,
					fire
				})
			)
		});

		demolition::t_defusing->Define
		({

		});

		demolition::ct_defusing->Define
		({
			After<Status::Enough>::With(look_c4, use)
		});

		demolition::ct_planted->Define
		({
			set_goal_c4,
			head_and_discard_goal
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
			Condition::If(CanUseHostage, rescue::lead_hostage)
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
			set_goal_vipsafety,
			head_and_discard_goal
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
			Priority::Create
			(
				{
					set_goal_tspawn,
					set_goal_ctspawn
				}
			),
			head_and_discard_goal
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
			head_and_discard_goal
		});
	}
}