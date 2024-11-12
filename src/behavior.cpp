#include "behavior.hpp"

namespace pokebot::bot {
	namespace behavior {
		std::shared_ptr<Action> breakpoint = Action::Create("breakpoint");

		namespace {
			template<bool b>
			bool IsSeeingEnemy(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->CanSeeEnemy());
			}

			template<bool b>
			bool HasPrimaryWeapon(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasPrimaryWeapon()));
			}

			template<bool b>
			bool HasSecondaryWeapon(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasSecondaryWeapon()));
			}

			template<bool b>
			bool HasMelee(const Bot* const Self) noexcept {
				return true;
			}

			template<bool b>
			bool HasGrenade(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasWeapon(game::Weapon::HEGrenade)));
			}

			template<bool b>
			bool HasFlashbang(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasWeapon(game::Weapon::Flashbang)));
			}

			template<bool b>
			bool HasSmoke(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasWeapon(game::Weapon::Smoke)));
			}

			template<bool b>
			bool IsEquipingArmour(const Bot* const Self) noexcept {
				return false;
			}

			template<bool b>
			bool IsEquipingHelmet(const Bot* const Self) noexcept {
				return false;
			}

			template<bool b>
			bool IsDying(const Bot* const Self) noexcept {
				RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->Health() <= 25));
			}

			template<bool b>
			bool IsLonely(const Bot* const Self) noexcept {
				return false;
			}

			template<bool b>
			bool HasEnoughPrimaryAmmo(const Bot* const Self) noexcept {
				return false;
			}

			template<bool b>
			bool HasEnoughSecondaryAmmo(const Bot* const Self) noexcept {
				return false;
			}
		}

		void DefineBehavior() {
			DefineCombat();
			DefineAction();
			DefineObjective();

			breakpoint->Define
			([](Bot* const) noexcept -> Status {
				return Status::Executed;
			});
		}
	}
}