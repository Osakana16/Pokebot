#include "behavior.hpp"

namespace pokebot::bot::behavior {
	namespace fight {
		std::shared_ptr<Priority> while_spotting_enemy = Priority::Create();
		std::shared_ptr<Sequence> beat_enemies = Sequence::Create();
		std::shared_ptr<Sequence> try_to_lose_sight = Sequence::Create();		// Try to lose the sight from the enemy

		std::shared_ptr<Priority> pick_best_weapon = Priority::Create();

		std::shared_ptr<Priority> while_losing_enemy = Priority::Create();
		std::shared_ptr<Sequence> try_to_find = Sequence::Create();

		std::shared_ptr<Sequence> flee = Sequence::Create();
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
				return !HasGuns(Self);
			} else {
				return HasGuns(Self);
			}
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
			fire
		});

		fight::try_to_lose_sight->Define
		({

		});

		fight::while_losing_enemy->Define
		({
			
		});

		fight::pick_best_weapon->Define
		({
			change_primary,
			change_secondary
		});
	}
}