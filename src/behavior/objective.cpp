#include "behavior.hpp"

namespace pokebot::bot::behavior {
	// - DEmolition Behaviors - 
	namespace demolition {
		std::shared_ptr<Priority> t_plant = Priority::Create("demolition::t_plant", Status::Executed);
		std::shared_ptr<Priority> t_planted_wary = Priority::Create("demolition::t_planted_wary", Status::Executed);
		std::shared_ptr<Priority> t_planted_camp = Priority::Create("demolition::t_planted_camp", Status::Executed);
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

	template<bool b>
	bool IsFarFromC4(const Bot* const Self) noexcept {
		assert(bot::manager.C4Origin().has_value());
		if constexpr (b) {
			return common::Distance(Self->Origin(), *bot::manager.C4Origin()) > 100.0f;
		} else {
			return common::Distance(Self->Origin(), *bot::manager.C4Origin()) <= 100.0f;
		}
	}

	void DefineObjective() {
		demolition::t_plant->Define
		({
			set_goal_bombspot,
			find_goal,
			After<Status::Enough>::With(
				head_to_goal, Priority::Create(
				{
					change_c4,
					fire
				})
			)
		});

		demolition::t_planted_wary->Define
		({
			Condition::If(IsFarFromC4<true>, set_goal_c4),
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

		demolition::ct_defusing->Define
		({
			After<Status::Enough>::With(look_c4, use)
		});

		demolition::ct_planted->Define
		({
			set_goal_c4,
			find_goal,
			head_to_goal
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
			Condition::If(IsHelper<false>, join_vip_squad)
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