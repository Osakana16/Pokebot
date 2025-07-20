export module pokebot.bot.behavior: behavior_definitions;
import :action;
import :selector;
import :sequence;
import :condition;
import :behavior_declaration;

import std;
import pokebot.bot;
import pokebot.game;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.terrain.graph.graph_base;
import pokebot.terrain.graph.node;
import pokebot.terrain.goal;
import pokebot.util;

#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT { return A; }
#define BEHAVIOR_IFELSE(AIF,A_PROCESS,BIF,B_PROCESS) Condition::If(AIF, A_PROCESS), Condition::If(BIF, B_PROCESS)
#define BEHAVIOR_IFELSE_TEMPLATE(IF,A_PROCESS,B_PROCESS) Condition::If(IF<true>, A_PROCESS), Condition::If(IF<false>, B_PROCESS)

#define RETURN_BEHAVIOR_TRUE_OR_FALSE(B,PROCESS) if constexpr (B) return (PROCESS); else return !(PROCESS)

#define BEHAVIOR_CREATE(TYPE,NAME) std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)


namespace pokebot::bot::behavior {
	namespace {
		template<bool b>
		bool IsSeeingEnemy(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			pokebot::util::PlayerName enemies_in_view[32]{};
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, enemies_in_view[0].size() >= 0);
		}

		template<bool b>
		bool HasPrimaryWeapon(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasPrimaryWeapon()));
		}

		template<bool b>
		bool HasSecondaryWeapon(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasSecondaryWeapon()));
		}

		template<bool b>
		bool HasMelee(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return true;
		}

		template<bool b>
		bool HasGrenade(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasWeapon(game::weapon::ID::HEGrenade)));
		}

		template<bool b>
		bool HasFlashbang(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasWeapon(game::weapon::ID::Flashbang)));
		}

		template<bool b>
		bool HasSmoke(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasWeapon(game::weapon::ID::Smoke)));
		}

		template<bool b>
		bool IsEquipingArmour(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsEquipingHelmet(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsDying(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->Health() <= 25));
		}

		template<bool b>
		bool IsLonely(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasEnoughPrimaryAmmo(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasEnoughSecondaryAmmo(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<node::GoalKind kind>
		Status SetGoal(Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			if (graph->IsOnNode(self->Origin(), self->goal_node) && graph->IsSameGoal(self->goal_node, kind))
				return Status::Success;

			node::NodeID id = node::Invalid_NodeID;
			auto goals = graph->GetNodeByKind(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				id = goal->second;
				break;
			}

			if (id != node::Invalid_NodeID && !graph->IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		}

		template<bool b>
		bool Always(const Bot* const, const node::Graph* const graph) POKEBOT_NOEXCEPT { return b; }

		template<game::Team value>
		bool Is(const Bot* const self, const game::Game* const game, const node::Graph* const graph) { return self->JoinedTeam() == value; }

		template<bool b>
		bool IsPlayingGame(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			return false;
		}

		template<bool b>
		bool CanSeeEnemy(const Bot* const self, const game::Game* const game, const node::Graph* const graph) { RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsLookingAtEnemy()); }

		template<bool b>
		bool IsBombPlanted(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, game->GetDemolitionManager()->GetC4Origin().has_value());
		}

		template<bool b>
		bool IsTickingToExplosion(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsOnBomb(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (game::Distance(self->Origin(), *game->GetDemolitionManager()->GetC4Origin()) <= 50.0f));
		}

		template<bool b>
		bool HasGoal(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->goal_node != node::Invalid_NodeID);
		}

		template<bool b>
		bool HasBomb(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->HasWeapon(game::weapon::ID::C4));
		}

		template<bool b>
		bool IsJumping(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, !self->IsOnFloor());
		}

		template<bool b>
		bool IsDucking(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsDucking());
		}

		template<bool b>
		bool IsSwimming(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsSwimming());
		}

		template<bool b>
		bool IsBlind(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, true);
		}

		template<bool b>
		bool IsUsing(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsPressingKey(game::player::ActionKey::Use));
		}

		template<bool b>
		bool IsDriving(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsDriving());
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
		bool IsCurrentMode(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return game->IsCurrentMode(flag);
		}

		template<bool b>
		bool IsHelper(const Bot* const) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsFeelingLikeBravely(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			return self->mood.brave >= 50;
		}

		template<bool b>
		bool IsFeelingLikeCooperation(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			return self->mood.coop >= 50;
		}

		template<bool b>
		bool IsVip(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			return false;
		}

		template<bool b, typename T>
		bool Is(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			static_assert(false);
			return false;
		}

		template<bool b, game::Team specified_team>
		bool Is(const Bot* const self, const game::Game* const game, const node::Graph* const graph) {
			return self->JoinedTeam() == specified_team;
		}
		BEHAVIOR_CREATE(Sequence, reset_goal_to_retreat);
		BEHAVIOR_CREATE(Selector, change_shotting_by_distance);

		bool HasGuns(const bot::Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return self->HasPrimaryWeapon() || self->HasSecondaryWeapon();
		}

		template<bool b>
		bool HasCrisis(const bot::Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			if constexpr (b) {
				return !self->IsGoodCondition();
			} else {
				return self->IsGoodCondition();
			}
		}

		template<bool b>
		bool IsEnemyFar(const bot::Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsEnemyFar());
		}
	}
}

export namespace pokebot::bot::behavior {
	void DefineAction();
	void DefineObjective();

	std::shared_ptr<Action> breakpoint = Action::Create("breakpoint");
;
		
	template<game::player::ActionKey key>
	Status BotPressesKey(Bot* const self, const game::Game* const game, const node::Graph* const graph) {
		if (!self->IsPressingKey(key)) {
			self->PressKey(key);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}
	
	template<node::GoalKind kind>
	Status BotSetsGoal(Bot* const self, const game::Game* const game, const node::Graph* const graph) {
		return SetGoal<kind>(self, game, graph);
	}

	template<game::weapon::ID weapon>
	Status BotChangesIfNotSelected(Bot* const self, const game::Game* const game, const node::Graph* const graph) noexcept {
		if (!self->IsCurrentWeapon(weapon)) {
			self->SelectWeapon(weapon);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}


	std::shared_ptr<Action> wait(std::uint32_t sec, float revision) {
		static std::unordered_map<std::uint32_t, std::shared_ptr<Action>> waits{};
		std::shared_ptr<Action>& wait = waits[sec];

		if (wait == nullptr) {
			wait = Action::Create(std::format("wait for {}", sec));
			wait->Define([sec, revision](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
				class Timer final {
				public:
					enum class Status {
						Not_Running,
						Running,
						Finished
					} RunningStatus() const POKEBOT_NOEXCEPT {
						if (time <= 0.0f) {
							return Status::Not_Running;
						} else if (time >= gpGlobals->time) {
							return Status::Running;
						} else {
							return Status::Finished;
						}
					}

					void SetTime(const float time_) POKEBOT_NOEXCEPT {
						time = time_ + gpGlobals->time;
					}
				private:
					float time{};
				};
				static std::unordered_map<pokebot::util::PlayerName, Timer, pokebot::util::PlayerName::Hash> timers{};
				auto& timer = timers[self->Name().data()];

				switch (timer.RunningStatus()) {
					case Timer::Status::Not_Running:
						timer.SetTime(sec + revision);
						[[fallthrough]];
						return Status::Failed;
					case Timer::Status::Running:
						return Status::Running;
					case Timer::Status::Finished:
						timer.SetTime(-99999.0);
						return Status::Success;
					default:
						assert(0);
				}
			});
		}
		return wait;
	}

	namespace demolition {
		std::shared_ptr<Sequence> plant = Sequence::Create("demolition::plant");
		std::shared_ptr<Sequence> defuse = Sequence::Create("demolition::defuse");
	}

	// - Rescue Behaviors -
	namespace rescue {
		std::shared_ptr<Sequence> head_to_hostage = Sequence::Create("head_to_hostage");
		auto get_closer_to_hostage = Selector::Create("use_to_rescue");
		auto use_to_rescue = Sequence::Create("use_to_rescue");
	}

	// - ASsasination Behaviors -
	namespace assassination {
		std::shared_ptr<Sequence> ct_cover = Sequence::Create("assassination::ct_cover");
		std::shared_ptr<Sequence> ct_take_point = Sequence::Create("assassination::ct_take_point");
		std::shared_ptr<Sequence> ct_vip_escape = Sequence::Create("assassination::ct_vip_escape");
	}
	
	// - EScape Behaviors -
	namespace escape {
		std::shared_ptr<Sequence> t_get_primary = Sequence::Create("escape::t_get_primary");
		std::shared_ptr<Sequence> t_take_point = Sequence::Create("escape::t_take_point");
	}

	std::shared_ptr<Sequence> t_ordinary = Sequence::Create("elimination::t_ordinary");
	std::shared_ptr<Sequence> ct_ordinary = Sequence::Create("elimination::ct_ordinary");

	BEHAVIOR_CREATE(Sequence, reset_team_objective);

	template<bool b>
	bool IsEnoughToRescueHostage(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		// Return true if the following conditions meet the requirements
		// 1. I have some hostages.
		// 2. Teammates are leading some hostages.
		if constexpr (b) {
			return false;
		} else {
			return true;
		}
	}
	
	template<bool b>
	bool IsTeamObjectiveSet(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		if constexpr (b) {
			return self->goal_node == self->GetTeamGoal() || self->goal_queue.Get() == self->GetTeamGoal() ;
		} else {
			return self->goal_node != self->GetTeamGoal() && self->goal_queue.Get() != self->GetTeamGoal();
		}
	}

	bool CanUseHostage(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		return false;
		// return const_cast<game::Game*>(game)->GetClosedHostage(self->Origin(), 83.0f) != nullptr;
	}

	template<bool b>
	bool IsFarFromC4(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		assert(game->GetDemolitionManager()->GetC4Origin().has_value());
		if constexpr (b) {
			return game::Distance(self->Origin(), *game->GetDemolitionManager()->GetC4Origin()) > 100.0f;
		} else {
			return game::Distance(self->Origin(), *game->GetDemolitionManager()->GetC4Origin()) <= 100.0f;
		}
	}

	template<bool b>
	bool IsFarFromMainGoal(const Bot* const self, const game::Game* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
#if 0
		auto id = Manager::Instance().GetGoalNode(self->Name().c_str());
		auto origin = graph->GetOrigin(id);
		auto source = self->Origin();
		if constexpr (b) {
			return game::Distance(source, *reinterpret_cast<Vector*>(&origin)) > 200.0f;
		} else {
			return game::Distance(source, *reinterpret_cast<Vector*>(&origin)) <= 200.0f;
		}
#else
		return false;
#endif
	}

	
	namespace fight {
		std::shared_ptr<Sequence> try_to_lose_sight = Sequence::Create("fight::try_to_lose_sight");		// Try to lose the sight from the enemy

		std::shared_ptr<Selector> pick_best_weapon = Selector::Create("fight::pick_best_weapon");

		std::shared_ptr<Selector> while_losing_enemy = Selector::Create("fight::while_losing_enemy");
		std::shared_ptr<Sequence> try_to_find = Sequence::Create("fight::try_to_find");

		std::shared_ptr<Sequence> flee = Sequence::Create("fight::flee");

		std::shared_ptr<Selector> decide_firing = Selector::Create("fight::decide_firing");

		std::shared_ptr<Selector> one_tap_fire = Selector::Create("fight::one_tap_fire");
		std::shared_ptr<Selector> full_burst_fire = Selector::Create("fight::full_burst_fire");

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

	 void DefineBehavior() {
		DefineCombat();
		DefineAction();
		DefineObjective();
	}
}