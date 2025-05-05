#include "behavior.hpp"
#include "bot/manager.hpp"

namespace pokebot::bot::behavior {
	namespace fight {
		std::shared_ptr<Sequence> try_to_lose_sight = Sequence::Create("fight::try_to_lose_sight");		// Try to lose the sight from the enemy

		std::shared_ptr<Priority> pick_best_weapon = Priority::Create("fight::pick_best_weapon");

		std::shared_ptr<Priority> while_losing_enemy = Priority::Create("fight::while_losing_enemy");
		std::shared_ptr<Sequence> try_to_find = Sequence::Create("fight::try_to_find");

		std::shared_ptr<Sequence> flee = Sequence::Create("fight::flee");
		
		std::shared_ptr<Priority> decide_firing = Priority::Create("fight::decide_firing");

		std::shared_ptr<Priority> one_tap_fire = Priority::Create("fight::one_tap_fire");
		std::shared_ptr<Priority> full_burst_fire = Priority::Create("fight::full_burst_fire");

	}
	

		template<bool b>
		bool Always(const Bot* const) POKEBOT_NOEXCEPT { return b; }

		template<game::Team value>
		bool Is(const Bot* const Self) { return Self->JoinedTeam() == value; }

		template<bool b>
		bool IsPlayingGame(const Bot* const Self) {
			return false;
		}
		
		template<bool b>
		bool CanSeeEnemy(const Bot* const Self) { RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsLookingAtEnemy()); }

		template<bool b>
		bool IsBombPlanted(const Bot* const Self) {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Manager::Instance().C4Origin().has_value());
		}

		template<bool b>
		bool IsTeamObjectiveSet(const Bot* const Self) POKEBOT_NOEXCEPT {
			if constexpr (b) {
				return Self->goal_node == Manager::Instance().GetGoalNode(Self->JoinedTeam(), Self->JoinedPlatoon());
			} else {
				return Self->goal_node != Manager::Instance().GetGoalNode(Self->JoinedTeam(), Self->JoinedPlatoon());
			}
		}

		template<bool b>
		bool IsTickingToExplosion(const Bot* const Self) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsOnBomb(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (game::Distance(Self->Origin(), *Manager::Instance().C4Origin()) <= 50.0f));
		}
		
		template<bool b>
		bool HasGoal(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->goal_node != node::Invalid_NodeID);
		}

		template<bool b>
		bool HasBomb(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->HasWeapon(game::Weapon::C4));
		}

		template<bool b>
		bool IsPlayerMate(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->JoinedTeam() == game::GetTeamFromModel(game::game.host.AsEdict())));
		}

		template<bool b>
		bool IsJumping(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, !Self->IsOnFloor());
		}

		template<bool b>
		bool IsDucking(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsDucking());
		}

		template<bool b>
		bool IsSwimming(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsSwimming());
		}

		template<bool b>
		bool IsBlind(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, true);
		}

		template<bool b>
		bool IsUsing(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsPressingKey(ActionKey::Use));
		}

		template<bool b>
		bool IsDriving(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsDriving());
		}

		template<bool b>
		bool IsInDark(const Bot* const) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasMoneyEnough(const Bot* const) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsTimeEarly(const Bot* const) POKEBOT_NOEXCEPT {
			return false;
		}

		template<game::MapFlags flag>
		bool IsCurrentMode(const Bot* const Self) POKEBOT_NOEXCEPT {
			return game::game.IsCurrentMode(flag);
		}

		template<bool b>
		bool IsHelper(const Bot* const) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsFeelingLikeBravely(const Bot* const Self) {
			return Self->mood.brave >= 50;
		}

		template<bool b>
		bool IsFeelingLikeCooperation(const Bot* const Self) {
			return Self->mood.coop >= 50;
		}

		template<bool b>
		bool IsVip(const Bot* const Self) {
			return false;
		}

		template<bool b, typename T>
		bool Is(const Bot* const Self) {
			static_assert(false);
			return false;
		}

		template<bool b, game::Team specified_team>
		bool Is(const Bot* const Self) {
			return Self->JoinedTeam() == specified_team;
		}

	namespace {
		BEHAVIOR_CREATE(Sequence, reset_goal_to_retreat);
		BEHAVIOR_CREATE(Priority, change_shotting_by_distance);

		bool HasGuns(const bot::Bot* const Self) POKEBOT_NOEXCEPT {
			return Self->HasPrimaryWeapon() || Self->HasSecondaryWeapon();
		}

		template<bool b>
		bool IsDying(const bot::Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->Health() <= 25));
		}

		template<bool b>
		bool HasCrisis(const bot::Bot* const Self) POKEBOT_NOEXCEPT {
			if constexpr (b) {
				return !Self->IsGoodCondition();
			} else {
				return Self->IsGoodCondition();
			}
		}

		template<bool b>
		bool IsEnemyFar(const bot::Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsEnemyFar());
		}
	}

	void DefineCombat() {
		reset_goal_to_retreat->Define
		({
			Condition::If(Is<true, game::Team::CT>, set_goal_ctspawn),
			Condition::If(Is<true, game::Team::T>, set_goal_tspawn)					  
		});

		fight::beat_enemies->Define
		({
			look_enemy,
			fight::pick_best_weapon,
			fight::decide_firing
		});

		fight::retreat->Define
		({
			Condition::If(IsTeamObjectiveSet<true>, reset_goal),
			Condition::If(HasGoal<false>, reset_goal_to_retreat),
			find_goal,
			head_to_goal
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

		change_shotting_by_distance->Define
		({
			Condition::If(IsEnemyFar<true>, fight::one_tap_fire),
			Condition::If(IsEnemyFar<false>, fight::full_burst_fire)
		 });

		fight::decide_firing->Define
		({
			change_shotting_by_distance
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