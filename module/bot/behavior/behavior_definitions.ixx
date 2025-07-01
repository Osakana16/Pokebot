export module pokebot.bot.behavior: behavior_definitions;
import :action;
import :priority;
import :sequence;
import :condition;
import :behavior_declaration;

import std;
import pokebot.bot;
import pokebot.game;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.terrain.graph;
import pokebot.terrain.graph.node;
import pokebot.terrain.goal;
import pokebot.util;

#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const Self) POKEBOT_NOEXCEPT { return A; }
#define BEHAVIOR_IFELSE(AIF,A_PROCESS,BIF,B_PROCESS) Condition::If(AIF, A_PROCESS), Condition::If(BIF, B_PROCESS)
#define BEHAVIOR_IFELSE_TEMPLATE(IF,A_PROCESS,B_PROCESS) Condition::If(IF<true>, A_PROCESS), Condition::If(IF<false>, B_PROCESS)

#define RETURN_BEHAVIOR_TRUE_OR_FALSE(B,PROCESS) if constexpr (B) return (PROCESS); else return !(PROCESS)

#define BEHAVIOR_CREATE(TYPE,NAME) std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)


namespace pokebot::bot::behavior {
	namespace {
		template<bool b>
		bool IsSeeingEnemy(const Bot* const Self) POKEBOT_NOEXCEPT {
			pokebot::util::PlayerName enemies_in_view[32]{};
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, enemies_in_view[0].size() >= 0);
		}

		template<bool b>
		bool HasPrimaryWeapon(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasPrimaryWeapon()));
		}

		template<bool b>
		bool HasSecondaryWeapon(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasSecondaryWeapon()));
		}

		template<bool b>
		bool HasMelee(const Bot* const Self) POKEBOT_NOEXCEPT {
			return true;
		}

		template<bool b>
		bool HasGrenade(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasWeapon(game::weapon::ID::HEGrenade)));
		}

		template<bool b>
		bool HasFlashbang(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasWeapon(game::weapon::ID::Flashbang)));
		}

		template<bool b>
		bool HasSmoke(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->HasWeapon(game::weapon::ID::Smoke)));
		}

		template<bool b>
		bool IsEquipingArmour(const Bot* const Self) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsEquipingHelmet(const Bot* const Self) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool IsDying(const Bot* const Self) POKEBOT_NOEXCEPT {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->Health() <= 25));
		}

		template<bool b>
		bool IsLonely(const Bot* const Self) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasEnoughPrimaryAmmo(const Bot* const Self) POKEBOT_NOEXCEPT {
			return false;
		}

		template<bool b>
		bool HasEnoughSecondaryAmmo(const Bot* const Self) POKEBOT_NOEXCEPT {
			return false;
		}

		template<node::GoalKind kind>
		Status SetGoal(Bot* const self) POKEBOT_NOEXCEPT {
			if (node::czworld.IsOnNode(self->Origin(), self->goal_node) && node::czworld.IsSameGoal(self->goal_node, kind))
				return Status::Success;

			node::NodeID id = node::Invalid_NodeID;
			auto goals = node::czworld.GetNodeByKind(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				id = goal->second;
				break;
			}

			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
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
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->HasWeapon(game::weapon::ID::C4));
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
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsPressingKey(game::player::ActionKey::Use));
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
		BEHAVIOR_CREATE(Sequence, reset_goal_to_retreat);
		BEHAVIOR_CREATE(Priority, change_shotting_by_distance);

		bool HasGuns(const bot::Bot* const Self) POKEBOT_NOEXCEPT {
			return Self->HasPrimaryWeapon() || Self->HasSecondaryWeapon();
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


		auto LookAt = [](Bot* const self, const Vector& Dest, const float Range) POKEBOT_NOEXCEPT->Status{
			self->look_direction.view = Dest;
			self->look_direction.movement = Dest;
			return Status::Success;
		};
	}
}

export namespace pokebot::bot::behavior {
	std::shared_ptr<Action> breakpoint = Action::Create("breakpoint");
;
		
	template<game::player::ActionKey key>
	Status BotPressesKey(Bot* const self) {
		if (!self->IsPressingKey(key)) {
			self->PressKey(key);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}
	
	template<node::GoalKind kind>
	Status BotSetsGoal(Bot* const self) {
		return SetGoal<kind>(self);
	}

	template<game::weapon::ID weapon>
	Status BotChangesIfNotSelected(Bot* const self) {
		if (!self->IsCurrentWeapon(weapon)) {
			self->SelectWeapon(weapon);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}

	void DefineAction() {
		reset_goal->Define([](Bot* const self) -> Status {
			if (self->goal_node == node::Invalid_NodeID && self->next_dest_node == node::Invalid_NodeID)
				return Status::Failed;

			self->goal_queue.Clear();
			self->next_dest_node = self->goal_node = node::Invalid_NodeID;
			self->routes.Clear();
			return Status::Success;
		});

		change_primary->Define([](Bot* const self) -> Status {
			if (self->HasPrimaryWeapon()) {
				self->SelectPrimaryWeapon();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		change_secondary->Define([](Bot* const self) -> Status {
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

		look_c4->Define([](Bot* const self) -> Status {
			return LookAt(self, *Manager::Instance().C4Origin() - Vector{ 0, 0, 36 }, 1.0f);
		});

		look_hostage->Define([](Bot* const self) -> Status {
			return Status::Failed;
		});

		look_enemy->Define([](Bot* const self) -> Status {
			if (self->HasEnemy()) {
				self->LookAtClosestEnemy();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		look_door->Define([](Bot* const self) -> Status {
			return Status::Failed;
		});

		look_button->Define([](Bot* const self) -> Status {
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

		set_goal_team_objective->Define([](Bot* const self) -> Status {
			const node::NodeID Node = Manager::Instance().GetGoalNode(self->Name().c_str());
			assert(Node != node::Invalid_NodeID);

			node::NodeID id = Node;
			if (node::czworld.IsOnNode(self->Origin(), id))
				return Status::Failed;

			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		rapid_fire->Define([](Bot* const self) -> Status {
			if (self->IsPressingKey(game::player::ActionKey::Attack)) {
				self->PressKey(game::player::ActionKey::Attack);
				return Status::Running;
			} else {
				self->PressKey(game::player::ActionKey::Attack);
				return Status::Success;
			}
		});

		lock->Define([](Bot* const self) -> Status {
			self->LockByBomb();
			return Status::Success;
		});

		set_goal_c4_node->Define([](Bot* const self) -> Status {
			auto area = node::czworld.GetNearest(*Manager::Instance().C4Origin());
			for (const auto& Another_Origin : { Vector{}, Vector{50.0f, 0.0f, 0.0f}, Vector{ -50.0f, 0.0f, 0.0f }, Vector{0.0f, 50.0f, 0.0f}, Vector{0.0f, -50.0f, 0.0f} }) {
				if ((area = node::czworld.GetNearest(*Manager::Instance().C4Origin() + Another_Origin)) != nullptr) {
					break;
				}
			}

			assert(area != nullptr);
			node::NodeID id = area->m_id;

			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		move_vector->Define([](Bot* const self) -> Status {
			if (!self->goal_vector.has_value())
				return Status::Failed;

			if (game::Distance(self->Origin(), *self->goal_vector) >= 5.0f) {
				self->look_direction.view = *self->goal_vector;
				self->look_direction.movement = *self->goal_vector;
				self->PressKey(game::player::ActionKey::Run);
			}
			return Status::Success;
		});

		set_goal_c4_vector->Define([](Bot* const self) -> Status {
			if (!Manager::Instance().C4Origin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *Manager::Instance().C4Origin();
			return Status::Success;
		});

		set_goal_hostage_vector->Define([](Bot* const self) -> Status {
			return Status::Success;
		});

		set_goal_backpack_node->Define([](Bot* const self) -> Status {
			auto area = node::czworld.GetNearest(*Manager::Instance().BackpackOrigin());
			if (area == nullptr) {
				return Status::Failed;
			}
			node::NodeID id = node::czworld.GetNearest(*Manager::Instance().BackpackOrigin())->m_id;
			if (node::czworld.IsOnNode(self->Origin(), id))
				return Status::Failed;

			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		set_goal_backpack_vector->Define([](Bot* const self) -> Status {
			if (!Manager::Instance().BackpackOrigin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *Manager::Instance().BackpackOrigin();
			return Status::Success;
		});

		set_goal_from_team_objective_within_range->Define([](Bot* const self) -> Status {
			auto findCircleLine = [self](const Vector& Origin, const float Distance) POKEBOT_NOEXCEPT -> node::NodeID {
				node::NodeID id = node::Invalid_NodeID;
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					auto area = node::czworld.GetNearest(*reinterpret_cast<Vector*>(&node::czworld.GetOrigin(bot::Manager::Instance().GetGoalNode(self->Name().c_str()))) + Line, FLT_MAX);
					if (area == nullptr)
						continue;

					id = area->m_id;
					if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(Origin, id))
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

		set_goal_from_c4_within_range->Define([](Bot* const self) -> Status {
			auto findCircleLine = [](const Vector& Origin, const float Distance) POKEBOT_NOEXCEPT -> node::NodeID {
				node::NodeID id = node::Invalid_NodeID;
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					auto area = node::czworld.GetNearest(*Manager::Instance().C4Origin() + Line, FLT_MAX);
					if (area == nullptr)
						continue;

					id = area->m_id;
					if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(Origin, id))
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

		set_goal_hostage_node->Define([](Bot* const self) -> Status {
			return Status::Failed;
		});

		set_goal_bombspot->Define(BotSetsGoal<node::GoalKind::Bombspot>);
		set_goal_rescuezone->Define(BotSetsGoal<node::GoalKind::Rescue_Zone>);
		set_goal_escapezone->Define(BotSetsGoal<node::GoalKind::Escape_Zone>);
		set_goal_vipsafety->Define(BotSetsGoal<node::GoalKind::Vip_Safety>);
		set_goal_tspawn->Define(BotSetsGoal<node::GoalKind::Terrorist_Spawn>);
		set_goal_ctspawn->Define(BotSetsGoal<node::GoalKind::CT_Spawn>);

		find_goal->Define([](Bot* const self) -> Status {
			if (self->routes.Empty() || self->routes.IsEnd()) {
				if (!self->goal_queue.IsEmpty()) {
					self->goal_node = self->goal_queue.Get();
				}

				if (auto area = node::czworld.GetNearest(self->Origin()); area != nullptr && area->m_id == self->goal_node) {
					// If the bot is on the goal node, he need not to find new path.
					return Status::Failed; 
				}
				// Find path.
				if (const auto Goal_Node_ID = self->goal_node; Goal_Node_ID != node::Invalid_NodeID) {
					auto hlorigin = *node::czworld.GetOrigin(0);
					Vector origin = *reinterpret_cast<Vector*>(&hlorigin);
					node::czworld.FindPath(&self->routes, self->Origin(), origin, self->JoinedTeam());
					if (self->routes.Empty() || self->routes.Destination() != self->goal_node) {
						return Status::Failed;
					} else {
						return Status::Success;
					}
				}
			}
			return Status::Failed;
		});

		head_to_goal->Define([](Bot* const self) -> Status {
			// Manage current node.
			
			if (!self->routes.Empty() && !self->routes.IsEnd()) {
				const auto Area = node::czworld.GetNearest(self->Origin());
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
				} else if (node::czworld.IsOnNode(self->Origin(), self->next_dest_node)) {
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

	std::shared_ptr<Action> wait(std::uint32_t sec, float revision) {
		static std::unordered_map<std::uint32_t, std::shared_ptr<Action>> waits{};
		std::shared_ptr<Action>& wait = waits[sec];

		if (wait == nullptr) {
			wait = Action::Create(std::format("wait for {}", sec));
			wait->Define([sec, revision](Bot* const self) -> Status {
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
		auto get_closer_to_hostage = Priority::Create("use_to_rescue");
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
	bool IsEnoughToRescueHostage(const Bot* const Self) POKEBOT_NOEXCEPT {
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
	bool IsTeamObjectiveSet(const Bot* const Self) POKEBOT_NOEXCEPT {
		if constexpr (b) {
			return Self->goal_node == Manager::Instance().GetGoalNode(Self->Name().c_str());
		} else {
			auto troops_goal_node = Manager::Instance().GetGoalNode(Self->Name().c_str());
			return Self->goal_node != troops_goal_node;
		}
	}

	bool CanUseHostage(const Bot* const Self) POKEBOT_NOEXCEPT {
		return game::game.GetClosedHostage(Self->Origin(), 83.0f) != nullptr;
	}

	template<bool b>
	bool IsFarFromC4(const Bot* const Self) POKEBOT_NOEXCEPT {
		assert(bot::Manager::Instance().C4Origin().has_value());
		if constexpr (b) {
			return game::Distance(Self->Origin(), *bot::Manager::Instance().C4Origin()) > 100.0f;
		} else {
			return game::Distance(Self->Origin(), *bot::Manager::Instance().C4Origin()) <= 100.0f;
		}
	}

	template<bool b>
	bool IsFarFromMainGoal(const Bot* const Self) POKEBOT_NOEXCEPT {
		auto id = Manager::Instance().GetGoalNode(Self->Name().c_str());
		auto origin = node::czworld.GetOrigin(id);
		auto source = Self->Origin();
		if constexpr (b) {
			return game::Distance(source, *reinterpret_cast<Vector*>(&origin)) > 200.0f;
		} else {
			return game::Distance(source, *reinterpret_cast<Vector*>(&origin)) <= 200.0f;
		}
	}

	void DefineObjective() {
		reset_team_objective->Define
		({
			reset_goal,
			set_goal_team_objective
		 });

		demolition::t_plant->Define
		({
			Condition::If(IsTeamObjectiveSet<false>, reset_team_objective),
			find_goal,
			head_to_goal,
			change_c4,
			demolition::plant
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