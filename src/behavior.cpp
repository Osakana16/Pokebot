#include "behavior.hpp"

namespace pokebot::bot {
	namespace behavior {
		std::shared_ptr<Priority> team = Priority::Create();
		std::shared_ptr<Priority> combat = Priority::Create();
		std::shared_ptr<Priority> mission = Priority::Create();
		std::shared_ptr<Priority> root = Priority::Create();

		namespace {
			std::shared_ptr<Priority> be_squad_leader = Priority::Create();
			std::shared_ptr<Priority> join_squad = Priority::Create();
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

			DefineAction();
			DefineObjective();
		}
	}

	void Bot::BehaviorUpdate() noexcept {
		behavior::root->Evalute(this);
	}

	void Bot::DecideBehavior() {

	}
}