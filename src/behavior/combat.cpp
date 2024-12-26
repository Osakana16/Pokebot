#include "behavior.hpp"

namespace pokebot::bot::behavior {
	namespace fight {
		std::shared_ptr<Priority> while_spotting_enemy = Priority::Create("fight::while_spotting_enemy");
		std::shared_ptr<Sequence> beat_enemies = Sequence::Create("fight::beat_enemies");
		std::shared_ptr<Sequence> try_to_lose_sight = Sequence::Create("fight::try_to_lose_sight");		// Try to lose the sight from the enemy

		std::shared_ptr<Priority> pick_best_weapon = Priority::Create("fight::pick_best_weapon");

		std::shared_ptr<Priority> while_losing_enemy = Priority::Create("fight::while_losing_enemy");
		std::shared_ptr<Sequence> try_to_find = Sequence::Create("fight::try_to_find");

		std::shared_ptr<Sequence> flee = Sequence::Create("fight::flee");
		
		std::shared_ptr<Priority> decide_firing = Priority::Create("fight::decide_firing");

		std::shared_ptr<Priority> one_tap_fire = Priority::Create("fight::one_tap_fire");
		std::shared_ptr<Priority> full_burst_fire = Priority::Create("fight::full_burst_fire");
	}

	namespace {
		bool HasGuns(const bot::Bot* const Self) noexcept {
			return Self->HasPrimaryWeapon() || Self->HasSecondaryWeapon();
		}

		template<bool b>
		bool IsDying(const bot::Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->Health() <= 25));
		}

		template<bool b>
		bool HasCrisis(const bot::Bot* const Self) noexcept {
			if constexpr (b) {
				return !Self->IsGoodCondition();
			} else {
				return Self->IsGoodCondition();
			}
		}

		template<bool b>
		bool IsEnemyFar(const bot::Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsEnemyFar());
		}
	}

	void DefineCombat() {
		fight::while_spotting_enemy->Define
		({
			Condition::If(HasCrisis<false>, fight::beat_enemies),
			Condition::If(HasCrisis<true>, fight::try_to_lose_sight)
		});

		fight::beat_enemies->Define
		({
			look_enemy,
			fight::pick_best_weapon,
			fight::decide_firing
		});

		fight::try_to_lose_sight->Define
		({
			set_goal_ctspawn,
			head_to_goal
		});

		fight::while_losing_enemy->Define
		({
			
		});

		fight::pick_best_weapon->Define
		({
			change_primary,
			change_secondary
		});

		fight::decide_firing->Define
		({
			Condition::If(IsEnemyFar<true>, fight::one_tap_fire),
			Condition::If(IsEnemyFar<false>, fight::full_burst_fire)
		});

		fight::one_tap_fire->Define
		({
			wait(1, .0f),
			tap_fire
		});

		fight::full_burst_fire->Define
		({
			wait(0, 0.1f),
			tap_fire
		});
	}
}