#include "behavior.hpp"

namespace pokebot::bot::behavior {
	namespace {
		template<node::GoalKind kind>
		Status SetGoal(Bot *const self) noexcept {
#if !USE_NAVMESH
			if (node::world.IsOnNode(self->Origin(), self->goal_node) && node::world.IsSameGoal(self->goal_node, kind))
				return Status::Enough;

			node::NodeID id = node::Invalid_NodeID;
			auto goals = node::world.GetGoal(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				id = goal->second;
				break;
			}
			
			if (id != node::Invalid_NodeID && !node::world.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Executed;
			} else
				return Status::Enough;
#else
			if (node::czworld.IsOnNode(self->Origin(), self->goal_node) && node::czworld.IsSameGoal(self->goal_node, kind))
				return Status::Enough;

			node::NodeID id = node::Invalid_NodeID;
			auto goals = node::czworld.GetGoal(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				id = goal->second;
				break;
			}
			
			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Executed;
			} else
				return Status::Enough;
#endif
		}
	}

	auto LookAt = [](Bot* const self, const Vector& Dest, const float Range) noexcept -> Status {
		if (self->IsLookingAt(Dest, Range)) {
			self->look_direction.view = Dest;
			return Status::Enough;
		} else {
			self->look_direction.view = Dest;
			return Status::Executed;
		}
	};

	BEHAVIOR_CREATE(Action, change_primary);
	BEHAVIOR_CREATE(Action, change_secondary);
	BEHAVIOR_CREATE(Action, change_melee);
	BEHAVIOR_CREATE(Action, change_grenade);
	BEHAVIOR_CREATE(Action, change_flashbang);
	BEHAVIOR_CREATE(Action, change_smoke);
	BEHAVIOR_CREATE(Action, change_c4);
	BEHAVIOR_CREATE(Action, look_c4);
	BEHAVIOR_CREATE(Action, look_hostage);
	BEHAVIOR_CREATE(Action, look_enemy);
	BEHAVIOR_CREATE(Action, look_door);
	BEHAVIOR_CREATE(Action, look_button);
	BEHAVIOR_CREATE(Action, use);
	BEHAVIOR_CREATE(Action, fire);
	BEHAVIOR_CREATE(Action, jump);
	BEHAVIOR_CREATE(Action, duck);
	BEHAVIOR_CREATE(Action, walk);
	BEHAVIOR_CREATE(Action, change_silencer);
	BEHAVIOR_CREATE(Action, adjust_scope);
	BEHAVIOR_CREATE(Action, set_goal_c4);
	BEHAVIOR_CREATE(Action, set_goal_hostage);
	BEHAVIOR_CREATE(Action, set_goal_bombspot);
	BEHAVIOR_CREATE(Action, set_goal_rescuezone);
	BEHAVIOR_CREATE(Action, set_goal_escapezone);
	BEHAVIOR_CREATE(Action, set_goal_vipsafety);
	BEHAVIOR_CREATE(Action, set_goal_tspawn);
	BEHAVIOR_CREATE(Action, set_goal_ctspawn);
	BEHAVIOR_CREATE(Action, set_goal_weapon);
	BEHAVIOR_CREATE(Action, head_to_goal);
	BEHAVIOR_CREATE(Action, discard_latest_goal);

	BEHAVIOR_CREATE(Action, create_lonely_squad);
	BEHAVIOR_CREATE(Action, create_offense_squad);
	BEHAVIOR_CREATE(Action, create_defense_squad);
	BEHAVIOR_CREATE(Action, create_vip_squad);
	BEHAVIOR_CREATE(Action, be_squad_leader);
	BEHAVIOR_CREATE(Action, follow_squad_leader);
	BEHAVIOR_CREATE(Action, join_vip_squad);
	BEHAVIOR_CREATE(Action, join_player_squad);
	BEHAVIOR_CREATE(Action, join_offense_squad);
	BEHAVIOR_CREATE(Action, join_defense_squad);
	BEHAVIOR_CREATE(Action, left_squad);

	std::shared_ptr<After<Status::Enough>> head_and_discard_goal = After<Status::Enough>::With(head_to_goal, discard_latest_goal);
	
	void DefineAction() {
		auto changeIfNotSelected = [](Bot* const self, const game::Weapon Target_Weapon) noexcept -> Status {
			if (!self->IsCurrentWeapon(Target_Weapon)) {
				self->SelectWeapon(Target_Weapon);
				return Status::Executed;
			} else
				return Status::Enough;
		};

		change_primary->Define(
			[](Bot* const self) -> Status {
				Status result = Status::Not_Ready;
				if (self->HasPrimaryWeapon()) {
					self->SelectPrimaryWeapon();
					result = Status::Executed;
				}
				return result;
			}
		);

		change_secondary->Define(
			[](Bot* const self) -> Status {
				Status result = Status::Not_Ready;
				if (self->HasSecondaryWeapon()) {
					self->SelectSecondaryWeapon();
					result = Status::Executed;
				}
				return result;
			}
		);

		change_melee->Define(
			[changeIfNotSelected](Bot* const self) -> Status {
				return changeIfNotSelected(self, game::Weapon::Knife);
			}
		);

		change_grenade->Define(
			[changeIfNotSelected](Bot* const self) -> Status {
				return changeIfNotSelected(self, game::Weapon::Flashbang);
			}
		);

		change_flashbang->Define(
			[changeIfNotSelected](Bot* const self) -> Status {
				return changeIfNotSelected(self, game::Weapon::Flashbang);
			}
		);

		change_smoke->Define(
			[changeIfNotSelected](Bot* const self) -> Status {
				return changeIfNotSelected(self, game::Weapon::Smoke);
			}
		);

		change_c4->Define(
			[changeIfNotSelected](Bot* const self) -> Status {
				return changeIfNotSelected(self, game::Weapon::C4);
			}
		);

		look_c4->Define(
			[](Bot* const self) -> Status {
				return LookAt(self, *manager.C4Origin() - Vector{ 0, 0, 36 }, 1.0f);
			}
		);

		look_hostage->Define(
			[](Bot* const self) -> Status {
				return Status::Executed;
			}
		);

		look_enemy->Define(
			[](Bot* const self) -> Status {
				if (!self->IsLookingAtEnemy()){
					self->LookAtClosestEnemy();
					return Status::Executed;
				} else {
					return Status::Enough;
				}
			}
		);

		look_door->Define(
			[](Bot* const self) -> Status {
				return Status::Executed;
			}
		);

		look_button->Define(
			[](Bot* const self) -> Status {
				return Status::Executed;
			}
		);

		auto BotPressesKey = [](Bot* const self, const bot::ActionKey Key) noexcept -> Status {
			if (!self->IsPressingKey(Key)) {
				self->PressKey(Key);
				return Status::Executed;
			} else {
				return Status::Enough;
			} 
		};

		use->Define(
			[BotPressesKey](Bot* const self) -> Status {
				return BotPressesKey(self, bot::ActionKey::Use);
			}
		);

		fire->Define(
			[BotPressesKey](Bot* const self) -> Status {
				return BotPressesKey(self, bot::ActionKey::Attack);
			}
		);

		jump->Define(
			[BotPressesKey](Bot* const self) -> Status {
				return BotPressesKey(self, bot::ActionKey::Jump);
			}
		);

		duck->Define(
			[BotPressesKey](Bot* const self) -> Status {
				return BotPressesKey(self, bot::ActionKey::Duck);
			}
		);

		walk->Define(
			[BotPressesKey](Bot* const self) -> Status {
				return BotPressesKey(self, bot::ActionKey::Shift);
			}
		);

		change_silencer->Define(
			[BotPressesKey](Bot* const self) -> Status {
				return BotPressesKey(self, bot::ActionKey::Attack2);
			}
		);

		adjust_scope->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Attack2);
				return Status::Executed;
			}
		);

		set_goal_c4->Define(
			[](Bot* const self) -> Status {
#if !USE_NAVMESH
				node::NodeID id = node::world.GetNearest(*manager.C4Origin());
				if (node::world.IsOnNode(self->Origin(), id))
					return Status::Enough;

				if (id != node::Invalid_NodeID && !node::world.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
					return Status::Executed;
				} else
					return Status::Failed;
#else
				node::NodeID id = node::czworld.GetNearest(*manager.C4Origin())->m_id;
				if (node::czworld.IsOnNode(self->Origin(), id))
					return Status::Enough;

				if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
					return Status::Executed;
				} else
					return Status::Failed;
#endif
			}
		);

		set_goal_hostage->Define(
			[](Bot* const self) -> Status {
#if !USE_NAVMESH
				edict_t* entity{};
				float min_distance = std::numeric_limits<float>::max();
				node::NodeID id = node::Invalid_NodeID;
				while ((entity = common::FindEntityByClassname(entity, "hostage_entity")) != nullptr) {
					if (float distance = common::Distance(self->Origin(), entity->v.origin);  id == node::Invalid_NodeID || min_distance > distance) {
						min_distance = distance;
						id = node::world.GetNearest(entity->v.origin);
					}
				}

				if (id != node::Invalid_NodeID) {
					self->goal_queue.AddGoalQueue(id, 1);
					return Status::Executed;
				} else
					return Status::Failed;
#else
				edict_t* entity{};
				float min_distance = std::numeric_limits<float>::max();
				node::NodeID id = node::Invalid_NodeID;
				while ((entity = common::FindEntityByClassname(entity, "hostage_entity")) != nullptr) {
					if (float distance = common::Distance(self->Origin(), entity->v.origin);  id == node::Invalid_NodeID || min_distance > distance) {
						min_distance = distance;
						id = node::czworld.GetNearest(entity->v.origin)->m_id;
					}
				}

				if (id != node::Invalid_NodeID) {
					self->goal_queue.AddGoalQueue(id, 1);
					return Status::Executed;
				} else
					return Status::Failed;
#endif
			}
		);

		set_goal_bombspot->Define(
			[](Bot* const self) -> Status {
				return SetGoal<node::GoalKind::Bombspot>(self);
			}
		);

		set_goal_rescuezone->Define(
			[](Bot* const self) -> Status {
				return SetGoal<node::GoalKind::Rescue_Zone>(self);
			}
		);

		set_goal_escapezone->Define(
			[](Bot* const self) -> Status {
				return SetGoal<node::GoalKind::Escape_Zone>(self);
			}
		);

		set_goal_vipsafety->Define(
			[](Bot* const self) -> Status {
				return SetGoal<node::GoalKind::Vip_Safety>(self);
			}
		);

		set_goal_tspawn->Define(
			[](Bot* const self) -> Status {
				return SetGoal<node::GoalKind::Terrorist_Spawn>(self);
			}
		);

		set_goal_ctspawn->Define(
			[](Bot* const self) -> Status {
				return SetGoal<node::GoalKind::CT_Spawn>(self);
			}
		);

		head_to_goal->Define(
			[](Bot* const self) -> Status {
#if !USE_NAVMESH
				const auto Current_Node_ID = node::world.GetNearest(self->Origin());
				if (self->goal_node == node::Invalid_NodeID && self->goal_queue.IsEmpty()) {
					if (!self->IsFollowing() && self->Objective_Goal_Node() == Current_Node_ID) {
						return Status::Enough;
					} else 
						return Status::Not_Ready;
				}
				
				if (Current_Node_ID != node::Invalid_NodeID && self->goal_node == Current_Node_ID) {
					return Status::Enough;
				} else if (node::world.IsOnNode(self->Origin(), self->goal_node)) {
					return Status::Enough;
				}

				if (!self->goal_queue.IsEmpty()) {
					self->goal_node = self->goal_queue.Get();
				}

				if (self->routes.Empty() || self->routes.IsEnd()) {
					// Find path.

					const auto Goal_Node_ID = self->goal_node;
					if (Current_Node_ID == Goal_Node_ID) {
						return Status::Executed;
					} else if (Current_Node_ID == node::Invalid_NodeID) {
						return Status::Failed;
					}

					node::world.FindPath(&self->routes, Current_Node_ID, Goal_Node_ID);
					if (self->routes.Empty() || self->routes.Destination() != self->goal_node) {
						return Status::Failed;
					}
				}

				if (self->next_dest_node == node::Invalid_NodeID) {
					self->next_dest_node = self->routes.Current();
				} else if (node::world.IsOnNode(self->Origin(), self->next_dest_node)) {
					if (!self->routes.IsEnd()) {
						if (self->routes.Next())
							self->next_dest_node = self->routes.Current();
					} else {
						// self->goal_queue.Pop();
					}
				} else {
					self->PressKey(ActionKey::Run);
				}
				return Status::Executed;
#else
				const auto Area = node::czworld.GetNearest(self->Origin());
				if (Area == nullptr) {
					return Status::Not_Ready;
				}
				const auto Current_Node_ID = Area->m_id;
				if (self->goal_node == node::Invalid_NodeID && self->goal_queue.IsEmpty()) {
					if (!self->IsFollowing() && self->Objective_Goal_Node() == Current_Node_ID) {
						return Status::Enough;
					} else
						return Status::Not_Ready;
				}
				
				if (Current_Node_ID != node::Invalid_NodeID && self->goal_node == Current_Node_ID) {
					return Status::Enough;
				} else if (node::czworld.IsOnNode(self->Origin(), self->goal_node)) {
					return Status::Enough;
				}

				if (!self->goal_queue.IsEmpty()) {
					self->goal_node = self->goal_queue.Get();
				}

				if (self->routes.Empty() || self->routes.IsEnd()) {
					// Find path.

					const auto Goal_Node_ID = self->goal_node;
					if (Current_Node_ID == Goal_Node_ID) {
						return Status::Executed;
					} else if (Current_Node_ID == node::Invalid_NodeID) {
						return Status::Failed;
					}

					node::czworld.FindPath(&self->routes, self->Origin(), node::czworld.GetOrigin(Goal_Node_ID));
					if (self->routes.Empty() || self->routes.Destination() != self->goal_node) {
						return Status::Failed;
					}
				}

				if (self->next_dest_node == node::Invalid_NodeID) {
					self->next_dest_node = self->routes.Current();
				} else if (node::czworld.IsOnNode(self->Origin(), self->next_dest_node)) {
					if (!self->routes.IsEnd()) {
						if (self->routes.Next())
							self->next_dest_node = self->routes.Current();
					} else {
						// self->goal_queue.Pop();
					}
				} else {
					self->PressKey(ActionKey::Run);
				}
				return Status::Executed;
#endif
			}
		);

		discard_latest_goal->Define
		(
			[](Bot* const self) -> Status {
				if (self->goal_queue.IsEmpty()) {
					self->goal_node = node::Invalid_NodeID;
					return Status::Enough;
				}

				self->goal_queue.Pop();
				return Status::Executed;
			}
		);

		follow_squad_leader->Define
		([](Bot* const self) -> Status {
#if !USE_NAVMESH
			auto leader = manager.GetSquadLeader(self->JoinedTeam(), self->squad);
			self->goal_queue.AddGoalQueue(node::world.GetNearest(leader->origin), 5);
			return Status::Executed;
#else
			auto leader = manager.GetSquadLeader(self->JoinedTeam(), self->squad);
			self->goal_queue.AddGoalQueue(node::czworld.GetNearest(leader->origin)->m_id, 5);
			return Status::Executed;
#endif
		 });

		create_lonely_squad->Define
		([](Bot* const self) -> Status {
			self->squad = bot::manager.SetupLonelySquad(self->Name().data());
			return Status::Executed;
		 });

		create_offense_squad->Define
		([](Bot* const self) -> Status {
			self->squad = bot::manager.SetupOffenseSquad(self->Name().data());
			return Status::Executed;
		 });

		create_defense_squad->Define
		([](Bot* const self) -> Status {
			self->squad = bot::manager.SetupDefenseSquad(self->Name().data());
			return Status::Executed;
		 });

		create_vip_squad->Define
		([](Bot* const self) -> Status {
			self->squad = bot::manager.SetupVipSquad(self->Name().data());
			return Status::Executed;
		 });

		join_player_squad->Define
		([](Bot* const self) -> Status {
			self->squad = bot::manager.JoinSquad(self->Name().data(), Policy::Player);

			return Status::Executed;
		 });

		join_defense_squad->Define
		([](Bot* const self) -> Status {
			self->squad = bot::manager.JoinSquad(self->Name().data(), Policy::Defense);
			return Status::Executed;
		 });

		left_squad->Define
		([](Bot* const self) -> Status {
			self->squad = -1;
			bot::manager.LeftSquad(self->Name().data());
			return Status::Executed;
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
					} RunningStatus() const noexcept {
						if (time <= 0.0f) {
							return Status::Not_Running;
						} else if (time >= gpGlobals->time) {
							return Status::Running;
						} else {
							return Status::Finished;
						}
					}

					void SetTime(const float time_) noexcept {
						time = time_ + gpGlobals->time;
					}
				private:
					float time{};
				};
				static std::unordered_map<std::string, Timer> timers{};
				auto& timer = timers[self->Name().data()];

				switch (timer.RunningStatus()) {
					case Timer::Status::Not_Running:
						timer.SetTime(sec + revision);
						[[fallthrough]];
						return Status::Not_Ready;
					case Timer::Status::Running:
						return Status::Executed;
					case Timer::Status::Finished:
						timer.SetTime(-99999.0);
						return Status::Enough;
					default:
						assert(0);
				}
			});
		}
		return wait;
	}
}