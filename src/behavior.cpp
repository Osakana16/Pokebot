#include "behavior.hpp"

namespace pokebot::bot {
	namespace behavior {
		std::shared_ptr<Priority> team = Priority::Create();
		std::shared_ptr<Priority> combat = Priority::Create();
		std::shared_ptr<Priority> mission = Priority::Create();
		std::shared_ptr<Priority> root = Priority::Create();
		std::shared_ptr<Action> breakpoint = Action::Create("breakpoint");

		namespace {
			std::shared_ptr<Priority> be_squad_leader = Priority::Create();
			std::shared_ptr<Priority> join_squad = Priority::Create();

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
			root->Define
			({
				Condition::If(IsJoinedSquad<false>, team),
				Condition::If(HasEnemy<true>, combat),
				Condition::If(Always<true>, mission)
			 });

			team->Define
			({
				Condition::If(BEHAVIOR_IF(IsFeelingLikeBravely<true>(Self) && IsEnoughSquadEstablished<false>(Self)), be_squad_leader),
			 });

			combat->Define
			({
				Condition::If(IsSeeingEnemy<true>, fight::while_spotting_enemy)
			 });

			mission->Define
			({
				Condition::If(
					IsFollower<true>,
					coop::objective
				),
				Condition::If(
					[](const Bot* const self) {
						return game::game.IsCurrentMode(game::MapFlags::Demolition);
					},
					demolition::objective
				),
				Condition::If(
					[](const Bot* const self) {
						return game::game.IsCurrentMode(game::MapFlags::HostageRescue);
					},
					rescue::objective
				),
				Condition::If(
					[](const Bot* const self) {
						return game::game.IsCurrentMode(game::MapFlags::Assassination);
					},
					assist::objective
				),
				Condition::If(
					[](const Bot* const self) {
						return game::game.IsCurrentMode(game::MapFlags::Escape);
					},
					escape::objective
				),
				elimination::objective
			 });

			be_squad_leader->Define
			({
				Condition::If(IsVip<true>, create_vip_squad),
				Condition::If(IsFeelingLikeCooperation<false>, create_lonely_squad),
				Condition::If(IsFeelingLikeBravely<true>, create_offense_squad),
				create_defense_squad
			 });

			join_squad->Define
			({
				Condition::If(IsPlayerMate<true>, join_player_squad),
				Condition::If(IsVipSquadEnoughJoined<false>, join_vip_squad),
				Condition::If(IsFeelingLikeBravely<true>, join_offense_squad)
			 });

			DefineCombat();
			DefineAction();
			DefineObjective();

			breakpoint->Define
			([](Bot* const) noexcept -> Status {
				return Status::Executed;
			});
		}
	}

	void Bot::BehaviorUpdate() noexcept {
		behavior::root->Evalute(this);
	}

	void Bot::DecideBehavior() {

	}
}