module pokebot.bot.behavior: behavior_definitions;
import pokebot.bot.behavior.node;

import std;
import pokebot.bot;
import pokebot.game;
import pokebot.game.scenario;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.terrain.graph.graph_base;
import pokebot.terrain.graph.node;
import pokebot.terrain.goal;
import pokebot.util;

#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT { return A; }
#define BEHAVIOR_IFELSE(AIF,A_PROCESS,BIF,B_PROCESS) Condition::If(AIF, A_PROCESS), Condition::If(BIF, B_PROCESS)
#define BEHAVIOR_IFELSE_TEMPLATE(IF,A_PROCESS,B_PROCESS) Condition::If(IF<true>, A_PROCESS), Condition::If(IF<false>, B_PROCESS)

#define RETURN_BEHAVIOR_TRUE_OR_FALSE(B,PROCESS) if constexpr (B) return (PROCESS); else return !(PROCESS)

#define BEHAVIOR_CREATE(TYPE,NAME) std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)


namespace pokebot::bot::behavior {
	BEHAVIOR_CREATE(Sequence, reset_team_objective);

	std::shared_ptr<Action> wait(std::uint32_t sec, float revision) {
		static std::unordered_map<std::uint32_t, std::shared_ptr<Action>> waits{};
		std::shared_ptr<Action>& wait = waits[sec];

		if (wait == nullptr) {
			wait = Action::Create(std::format("wait for {}", sec));
			wait->Define([sec, revision](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
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

	namespace {
		template<bool b>
		bool IsSeeingEnemy(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			pokebot::util::PlayerName enemies_in_view[32]{};
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, enemies_in_view[0].size() >= 0);
		}

		template<bool b>
		bool HasPrimaryWeapon(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasPrimaryWeapon()));
		}

		template<bool b>
		bool HasSecondaryWeapon(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasSecondaryWeapon()));
		}

		template<bool b>
		bool HasMelee(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return true;
		}

		template<bool b>
		bool HasGrenade(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasWeapon(game::weapon::ID::HEGrenade)));
		}

		template<bool b>
		bool HasFlashbang(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasWeapon(game::weapon::ID::Flashbang)));
		}

		template<bool b>
		bool HasSmoke(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->HasWeapon(game::weapon::ID::Smoke)));
		}

		template<bool b>
		bool IsEquipingArmour(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsEquipingHelmet(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsDying(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (self->Health() <= 25));
		}

		template<bool b>
		bool IsLonely(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasEnoughPrimaryAmmo(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasEnoughSecondaryAmmo(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<node::GoalKind kind>
		Status SetGoal(Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
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
		bool Is(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) { return self->JoinedTeam() == value; }

		template<bool b>
		bool IsPlayingGame(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			return false;
		}

		template<bool b>
		bool CanSeeEnemy(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) { RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsLookingAtEnemy()); }

		template<bool b>
		bool IsBombPlanted(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, de_manager->GetC4Origin().has_value());
		}

		template<bool b>
		bool IsTickingToExplosion(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsOnBomb(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (game::Distance(self->Origin(), *de_manager->GetC4Origin()) <= 50.0f));
		}

		template<bool b>
		bool HasGoal(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->goal_node != node::Invalid_NodeID);
		}

		template<bool b>
		bool HasBomb(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->HasWeapon(game::weapon::ID::C4));
		}

		template<bool b>
		bool IsJumping(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, !self->IsOnFloor());
		}

		template<bool b>
		bool IsDucking(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsDucking());
		}

		template<bool b>
		bool IsSwimming(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsSwimming());
		}

		template<bool b>
		bool IsBlind(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, true);
		}

		template<bool b>
		bool IsUsing(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsPressingKey(game::player::ActionKey::Use));
		}

		template<bool b>
		bool IsDriving(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
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
		bool IsCurrentMode(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return game->GetScenario() & flag;
		}

		template<bool b>
		bool IsHelper(const Bot* const) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsFeelingLikeBravely(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			return self->mood.brave >= 50;
		}

		template<bool b>
		bool IsFeelingLikeCooperation(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			return self->mood.coop >= 50;
		}

		template<bool b>
		bool IsVip(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			return false;
		}

		template<bool b, typename T>
		bool Is(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			static_assert(false);
			return false;
		}

		template<bool b, game::Team specified_team>
		bool Is(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
			return self->JoinedTeam() == specified_team;
		}

		BEHAVIOR_CREATE(Sequence, reset_goal_to_retreat);
		BEHAVIOR_CREATE(Selector, change_shotting_by_distance);

		bool HasGuns(const bot::Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			return self->HasPrimaryWeapon() || self->HasSecondaryWeapon();
		}

		template<bool b>
		bool HasCrisis(const bot::Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			if constexpr (b) {
				return !self->IsGoodCondition();
			} else {
				return self->IsGoodCondition();
			}
		}

		template<bool b>
		bool IsEnemyFar(const bot::Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, self->IsEnemyFar());
		}
	}
}

namespace pokebot::bot::behavior {
	std::shared_ptr<Action> breakpoint = Action::Create("breakpoint");

	template<game::player::ActionKey key>
	Status BotPressesKey(Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
		if (!self->IsPressingKey(key)) {
			self->PressKey(key);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}

	template<node::GoalKind kind>
	Status BotSetsGoal(Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) {
		return SetGoal<kind>(self, game, graph);
	}

	template<game::weapon::ID weapon>
	Status BotChangesIfNotSelected(Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) noexcept {
		if (!self->IsCurrentWeapon(weapon)) {
			self->SelectWeapon(weapon);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}

	template<bool b>
	bool IsEnoughToRescueHostage(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
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
	bool IsTeamObjectiveSet(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		if constexpr (b) {
			return self->goal_node == self->GetTeamGoal() || self->goal_queue.Get() == self->GetTeamGoal();
		} else {
			return self->goal_node != self->GetTeamGoal() && self->goal_queue.Get() != self->GetTeamGoal();
		}
	}

	bool CanUseHostage(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		return false;
		// return const_cast<game::CSGameBase*>(game)->GetClosedHostage(self->Origin(), 83.0f) != nullptr;
	}

	template<bool b>
	bool IsFarFromC4(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
		auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
		assert(de_manager->GetC4Origin().has_value());
		if constexpr (b) {
			return game::Distance(self->Origin(), *de_manager->GetC4Origin()) > 100.0f;
		} else {
			return game::Distance(self->Origin(), *de_manager->GetC4Origin()) <= 100.0f;
		}
	}

	template<bool b>
	bool IsFarFromMainGoal(const Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) POKEBOT_NOEXCEPT {
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

	void DefineObjective() {
		reset_team_objective->Define
		({
			reset_goal,
			set_goal_team_objective
		 });

		demolition::t_plant->Define
		({
			Condition::If(
				IsTeamObjectiveSet<false>,
				Sequence::Create({
					Succeeder::As(reset_team_objective),
					Succeeder::As(set_goal_team_objective)
				}),
				Selector::Create({
					find_goal,			// 
					head_to_goal,		// 
					change_c4,			// 
					demolition::plant	// 
				})
			)
		 });

		demolition::plant->Define
		({
			lock,
			rapid_fire
		 });

		demolition::t_pick_up_bomb->Define
		({
			set_goal_backpack_node,
			find_goal,
			head_to_goal,
			set_goal_backpack_vector,
			move_vector
		 });

		demolition::defuse->Define
		({
			lock,
			use
		 });

		demolition::t_planted_wary->Define
		({
			Condition::If(IsFarFromC4<true>, set_goal_c4_node),
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

		demolition::ct_defend->Define
		({
			demolition::ct_defend_wary
		 });

		demolition::ct_defend_wary->Define
		({
			Condition::If(IsFarFromMainGoal<true>, set_goal_team_objective),
			Condition::If(IsFarFromMainGoal<false>, set_goal_from_team_objective_within_range),
			find_goal,
			head_to_goal
		 });

		demolition::ct_defend_camp->Define
		({

		 });

		demolition::ct_defusing->Define
		({
			look_c4,
			demolition::defuse
		 });

		demolition::ct_planted->Define
		({
			set_goal_c4_node,
			find_goal,
			head_to_goal,
			set_goal_c4_vector,
			move_vector
		 });

		/*
			Hostage Rescue
		*/

		rescue::t_defend_hostage->Define
		({
			Condition::If(IsFarFromMainGoal<true>, set_goal_team_objective),
			Condition::If(IsFarFromMainGoal<false>, set_goal_from_team_objective_within_range),
			find_goal,
			head_to_goal,
		 });

		rescue::ct_leave->Define
		({
			set_goal_rescuezone,
			find_goal,
			head_to_goal
		 });

		/*
			CT: Try to rescue a hostage.


		*/
		rescue::ct_try->Define
		({
			set_goal_hostage_node,
			find_goal,
			head_to_goal,
			rescue::use_to_rescue
		 });

		rescue::use_to_rescue->Define
		({
			rescue::get_closer_to_hostage,
			Condition::If(CanUseHostage, rescue::lead_hostage)
		 });

		rescue::get_closer_to_hostage->Define
		({
			set_goal_hostage_vector,
			move_vector
		 });

		rescue::lead_hostage->Define
		({
			look_hostage,
			wait(1, .0f),
			use
		 });

		assassination::ct_cover->Define
		({

		 });

		assassination::ct_take_point->Define
		({
			set_goal_vipsafety,
			find_goal,
			head_to_goal
		 });

		assassination::ct_vip_escape->Define
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

		t_ordinary->Define
		({
			Selector::Create
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
			Selector::Create
			(
				{
					set_goal_tspawn,
					set_goal_ctspawn
				}
			),
			head_to_goal
		 });
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

	void DefineAction() {
		auto LookAt = [](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph, const Vector& Dest, const float Range) POKEBOT_NOEXCEPT->Status{
			self->look_direction.view = Dest;
			self->look_direction.movement = Dest;
			return Status::Success;
		};

		reset_goal->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (self->goal_node == node::Invalid_NodeID && self->next_dest_node == node::Invalid_NodeID)
				return Status::Failed;

			self->goal_queue.Clear();
			self->next_dest_node = self->goal_node = node::Invalid_NodeID;
			self->routes.Clear();
			return Status::Success;
		});

		change_primary->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (self->HasPrimaryWeapon()) {
				self->SelectPrimaryWeapon();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		change_secondary->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (self->HasSecondaryWeapon()) {
				self->SelectSecondaryWeapon();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		change_melee->Define(BotChangesIfNotSelected<game::weapon::ID::Knife>);
		change_grenade->Define(BotChangesIfNotSelected<game::weapon::ID::HEGrenade>);
		change_flashbang->Define(BotChangesIfNotSelected<game::weapon::ID::Flashbang>);
		change_smoke->Define(BotChangesIfNotSelected<game::weapon::ID::Smoke>);
		change_c4->Define(BotChangesIfNotSelected<game::weapon::ID::C4>);

		look_c4->Define([LookAt](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			return LookAt(self, game, graph, *de_manager->GetC4Origin() - Vector{ 0, 0, 36 }, 1.0f);
		});

		look_hostage->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		look_enemy->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (self->HasEnemy()) {
				self->LookAtClosestEnemy();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		look_door->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		look_button->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		move_forward->Define(BotPressesKey<game::player::ActionKey::Run>);
		use->Define(BotPressesKey<game::player::ActionKey::Use>);
		tap_fire->Define(BotPressesKey<game::player::ActionKey::Attack>);
		jump->Define(BotPressesKey<game::player::ActionKey::Jump>);
		duck->Define(BotPressesKey<game::player::ActionKey::Duck>);
		walk->Define(BotPressesKey<game::player::ActionKey::Shift>);
		change_silencer->Define(BotPressesKey<game::player::ActionKey::Attack2>);
		adjust_scope->Define(BotPressesKey<game::player::ActionKey::Attack2>);

		set_goal_team_objective->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			const node::NodeID Node = self->GetTeamGoal();
			assert(Node != node::Invalid_NodeID);

			node::NodeID id = Node;
			if (graph->IsOnNode(self->Origin(), id))
				return Status::Failed;

			if (id != node::Invalid_NodeID && !graph->IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		rapid_fire->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (self->IsPressingKey(game::player::ActionKey::Attack)) {
				self->PressKey(game::player::ActionKey::Attack);
				return Status::Running;
			} else {
				self->PressKey(game::player::ActionKey::Attack);
				return Status::Success;
			}
		});

		lock->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			self->LockByBomb();
			return Status::Success;
		});

		set_goal_c4_node->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			auto area = graph->GetNearest(*de_manager->GetC4Origin());
			for (const auto& Another_Origin : { Vector{}, Vector{50.0f, 0.0f, 0.0f}, Vector{ -50.0f, 0.0f, 0.0f }, Vector{0.0f, 50.0f, 0.0f}, Vector{0.0f, -50.0f, 0.0f} }) {
				if ((area = graph->GetNearest(*de_manager->GetC4Origin() + Another_Origin)) != nullptr) {
					break;
				}
			}

			assert(area != nullptr);
			node::NodeID id = area->m_id;

			if (id != node::Invalid_NodeID && !graph->IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		move_vector->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (!self->goal_vector.has_value())
				return Status::Failed;

			if (game::Distance(self->Origin(), *self->goal_vector) >= 5.0f) {
				self->look_direction.view = *self->goal_vector;
				self->look_direction.movement = *self->goal_vector;
				self->PressKey(game::player::ActionKey::Run);
			}
			return Status::Success;
		});

		set_goal_c4_vector->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			if (!de_manager->GetC4Origin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *de_manager->GetC4Origin();
			return Status::Success;
		});

		set_goal_hostage_vector->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			return Status::Success;
		});

		set_goal_backpack_node->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			auto area = graph->GetNearest(*de_manager->GetBackpackOrigin());
			if (area == nullptr) {
				return Status::Failed;
			}
			node::NodeID id = graph->GetNearest(*de_manager->GetBackpackOrigin())->m_id;
			if (graph->IsOnNode(self->Origin(), id))
				return Status::Failed;

			if (id != node::Invalid_NodeID && !graph->IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		set_goal_backpack_vector->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
			if (!de_manager->GetBackpackOrigin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *de_manager->GetBackpackOrigin();
			return Status::Success;
		});

		set_goal_from_team_objective_within_range->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto findCircleLine = [self, graph](const Vector& Origin, const float Distance) POKEBOT_NOEXCEPT->node::NodeID{
				node::NodeID id = node::Invalid_NodeID;
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					// auto area = graph->GetNearest(*reinterpret_cast<Vector*>(&graph->GetOrigin(bot::Manager::Instance().GetGoalNode(self->Name().c_str()))) + Line, FLT_MAX);
					auto area = graph->GetNearest({});
					if (area == nullptr)
						continue;

					id = area->m_id;
					if (id != node::Invalid_NodeID && !graph->IsOnNode(Origin, id))
						return id;
				}
				return node::Invalid_NodeID;
			};

			std::queue<std::future<node::NodeID>> results{};
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 2000.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 1500.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 1000.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 3000.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 2500.0f));

			assert(!results.empty());

			while (!results.empty()) {
				auto id_result = results.front().get();
				if (id_result != node::Invalid_NodeID) {
					if (self->goal_queue.AddGoalQueue(id_result, 1)) {
						return Status::Success;
					}
				}
				results.pop();
			}
			return Status::Failed;
		});

		set_goal_from_c4_within_range->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			auto findCircleLine = [&](const Vector& Origin, const float Distance) POKEBOT_NOEXCEPT->node::NodeID{
				node::NodeID id = node::Invalid_NodeID;
				auto de_manager = std::static_pointer_cast<game::scenario::DemolitionManager>(game->GetDemolitionManager());
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					auto area = graph->GetNearest(*de_manager->GetC4Origin() + Line, FLT_MAX);
					if (area == nullptr)
						continue;

					id = area->m_id;
					if (id != node::Invalid_NodeID && !graph->IsOnNode(Origin, id))
						return id;
				}
				return node::Invalid_NodeID;
			};

			std::queue<std::future<node::NodeID>> results{};
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 2000.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 1500.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 1000.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 3000.0f));
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 2500.0f));

			assert(!results.empty());

			while (!results.empty()) {
				auto id_result = results.front().get();
				if (id_result != node::Invalid_NodeID) {
					if (self->goal_queue.AddGoalQueue(id_result, 1)) {
						return Status::Success;
					}
				}
				results.pop();
			}
			return Status::Failed;
		});

		set_goal_hostage_node->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		set_goal_bombspot->Define(BotSetsGoal<node::GoalKind::Bombspot>);
		set_goal_rescuezone->Define(BotSetsGoal<node::GoalKind::Rescue_Zone>);
		set_goal_escapezone->Define(BotSetsGoal<node::GoalKind::Escape_Zone>);
		set_goal_vipsafety->Define(BotSetsGoal<node::GoalKind::Vip_Safety>);
		set_goal_tspawn->Define(BotSetsGoal<node::GoalKind::Terrorist_Spawn>);
		set_goal_ctspawn->Define(BotSetsGoal<node::GoalKind::CT_Spawn>);

		find_goal->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			if (self->routes.Empty() || self->routes.IsEnd()) {
				if (!self->goal_queue.IsEmpty()) {
					self->goal_node = self->goal_queue.Get();
				}

				if (auto area = graph->GetNearest(self->Origin()); area != nullptr && area->m_id == self->goal_node) {
					// If the bot is on the goal node, he need not to find new path.
					return Status::Failed;
				}
				// Find path.
				if (const auto Goal_Node_ID = self->goal_node; Goal_Node_ID != node::Invalid_NodeID) {
					auto hlorigin = *graph->GetOrigin(Goal_Node_ID);
					Vector origin = *reinterpret_cast<Vector*>(&hlorigin);
					const_cast<node::Graph*>(graph)->FindPath(&self->routes, self->Origin(), origin, self->JoinedTeam());
					if (self->routes.Empty() || self->routes.Destination() != self->goal_node) {
						return Status::Failed;
					} else {
						return Status::Success;
					}
				}
			}
			return Status::Failed;
		});

		head_to_goal->Define([](Bot* const self, const game::CSGameBase* const game, const node::Graph* const graph) -> Status {
			// Manage current node.

			if (!self->routes.Empty() && !self->routes.IsEnd()) {
				const auto Area = graph->GetNearest(self->Origin());
				if (Area == nullptr) {
					return Status::Running;
				}

				// - Check -
				const auto Current_Node_ID = Area->m_id;
				if (!self->IsFollowing() && self->goal_node == Current_Node_ID) {
					// The bot is already reached at the destination.
					self->goal_queue.Clear();
					self->routes.Clear();
					self->goal_node = node::Invalid_NodeID;
					return Status::Success;
				}

				if (self->next_dest_node == node::Invalid_NodeID) {
					self->next_dest_node = self->routes.Current();
				} else if (graph->IsOnNode(self->Origin(), self->next_dest_node)) {
					if (!self->routes.IsEnd()) {
						if (self->routes.Next()) {
							self->next_dest_node = self->routes.Current();
						}
					}
				}
				return Status::Running;
			}
			return Status::Failed;
		});
	}

	void DefineBehavior() {
		DefineCombat();
		DefineAction();
		DefineObjective();
	}
}