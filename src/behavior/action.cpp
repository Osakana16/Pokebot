#include "behavior.hpp"
#include <future>

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
				return Status::Success;

			node::NodeID id = node::Invalid_NodeID;
			auto goals = node::czworld.GetGoal(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				id = goal->second;
				break;
			}
			
			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
#endif
		}
	}

	auto LookAt = [](Bot* const self, const Vector& Dest, const float Range) noexcept -> Status {
		self->look_direction.view = Dest;
		self->look_direction.movement = Dest;
		return Status::Success;
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
	BEHAVIOR_CREATE(Action, move_forward);
	BEHAVIOR_CREATE(Action, use);
	BEHAVIOR_CREATE(Action, tap_fire);
	BEHAVIOR_CREATE(Action, jump);
	BEHAVIOR_CREATE(Action, duck);
	BEHAVIOR_CREATE(Action, walk);
	BEHAVIOR_CREATE(Action, change_silencer);
	BEHAVIOR_CREATE(Action, adjust_scope);
	BEHAVIOR_CREATE(Action, set_goal_team_objective);
	BEHAVIOR_CREATE(Action, set_goal_from_c4_within_range);
	BEHAVIOR_CREATE(Action, set_goal_hostage);
	BEHAVIOR_CREATE(Action, set_goal_bombspot);
	BEHAVIOR_CREATE(Action, set_goal_rescuezone);
	BEHAVIOR_CREATE(Action, set_goal_escapezone);
	BEHAVIOR_CREATE(Action, set_goal_vipsafety);
	BEHAVIOR_CREATE(Action, set_goal_tspawn);
	BEHAVIOR_CREATE(Action, set_goal_ctspawn);
	BEHAVIOR_CREATE(Action, set_goal_weapon);
	BEHAVIOR_CREATE(Action, find_goal);
	BEHAVIOR_CREATE(Action, head_to_goal);
	
	template<ActionKey key>
	Status BotPressesKey(Bot* const self) {
		if (!self->IsPressingKey(key)) {
			self->PressKey(key);
			return Status::Success;
		} else {
			return Status::Failed;
		}
	}

	void DefineAction() {
		auto changeIfNotSelected = [](Bot* const self, const game::Weapon Target_Weapon) noexcept -> Status {
			if (!self->IsCurrentWeapon(Target_Weapon)) {
				self->SelectWeapon(Target_Weapon);
				return Status::Success;
			} else {
				return Status::Failed;
			}
		};

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

		change_melee->Define([changeIfNotSelected](Bot* const self) -> Status {
			return changeIfNotSelected(self, game::Weapon::Knife);
		});

		change_grenade->Define([changeIfNotSelected](Bot* const self) -> Status {
			return changeIfNotSelected(self, game::Weapon::Flashbang);
		});

		change_flashbang->Define([changeIfNotSelected](Bot* const self) -> Status {
			return changeIfNotSelected(self, game::Weapon::Flashbang);
		});

		change_smoke->Define([changeIfNotSelected](Bot* const self) -> Status {
			return changeIfNotSelected(self, game::Weapon::Smoke);
		});

		change_c4->Define([changeIfNotSelected](Bot* const self) -> Status {
			return changeIfNotSelected(self, game::Weapon::C4);
		});

		look_c4->Define([](Bot* const self) -> Status {
			return LookAt(self, *manager.C4Origin() - Vector{ 0, 0, 36 }, 1.0f);
		});

		look_hostage->Define([](Bot* const self) -> Status {
			return Status::Failed;
		});

		look_enemy->Define([](Bot* const self) -> Status {
			if (!self->IsLookingAtEnemy()) {
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

		move_forward->Define(BotPressesKey<bot::ActionKey::Run>);
		use->Define(BotPressesKey<bot::ActionKey::Use>);
		tap_fire->Define(BotPressesKey<bot::ActionKey::Attack>);
		jump->Define(BotPressesKey<bot::ActionKey::Jump>);
		duck->Define(BotPressesKey<bot::ActionKey::Duck>);
		walk->Define(BotPressesKey<bot::ActionKey::Shift>);
		change_silencer->Define(BotPressesKey<bot::ActionKey::Attack2>);
		adjust_scope->Define(BotPressesKey<bot::ActionKey::Attack2>);

		set_goal_team_objective->Define([](Bot* const self) -> Status {
			assert(manager.GetGoalNode(self->JoinedTeam(), self->JoinedPlatoon()) != node::Invalid_NodeID);
			node::NodeID id = manager.GetGoalNode(self->JoinedTeam(), self->JoinedPlatoon());
			if (node::czworld.IsOnNode(self->Origin(), id))
				return Status::Failed;

			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		rapid_fire->Define([](Bot* const self) -> Status {
			if (self->IsPressingKey(ActionKey::Attack)) {
				self->PressKey(ActionKey::Attack);
				return Status::Running;
			} else {
				self->PressKey(ActionKey::Attack);
				return Status::Success;
			}
		});

		set_goal_c4_node->Define([](Bot* const self) -> Status {
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
				return Status::Failed;

			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
#endif
		});

		move_vector->Define([](Bot* const self) -> Status {
			if (!self->goal_vector.has_value())
				return Status::Failed;

			if (common::Distance(self->Origin(), *self->goal_vector) <= 75.0f) {
				self->look_direction.view = *self->goal_vector;
				self->look_direction.movement = *self->goal_vector;
				self->PressKey(ActionKey::Run);
			}
			return Status::Success;
		});

		set_goal_c4_vector->Define([](Bot* const self) -> Status {
			if (!manager.C4Origin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *manager.C4Origin();
			return Status::Success;
		});

		set_goal_from_team_objective_within_range->Define([](Bot* const self) -> Status {
			auto findCircleLine = [self](const Vector& Origin, const float Distance) noexcept -> node::NodeID {
				node::NodeID id = node::Invalid_NodeID;
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					auto area = node::czworld.GetNearest(node::czworld.GetOrigin(bot::manager.GetGoalNode(self->JoinedTeam(), self->JoinedPlatoon())) + Line);
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
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 2500.0f));

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
			auto findCircleLine = [](const Vector& Origin, const float Distance) noexcept -> node::NodeID {
				node::NodeID id = node::Invalid_NodeID;
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					auto area = node::czworld.GetNearest(*manager.C4Origin() + Line);
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
			results.push(std::async(std::launch::async, findCircleLine, self->Origin(), 2500.0f));

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

		set_goal_hostage->Define([](Bot* const self) -> Status {
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
				return Status::Success;
			} else
				return Status::Failed;
#endif
		});

		set_goal_bombspot->Define([](Bot* const self) -> Status {
			return SetGoal<node::GoalKind::Bombspot>(self);
		});

		set_goal_rescuezone->Define([](Bot* const self) -> Status {
			return SetGoal<node::GoalKind::Rescue_Zone>(self);
		});

		set_goal_escapezone->Define([](Bot* const self) -> Status {
			return SetGoal<node::GoalKind::Escape_Zone>(self);
		});

		set_goal_vipsafety->Define([](Bot* const self) -> Status {
			return SetGoal<node::GoalKind::Vip_Safety>(self);
		});

		set_goal_tspawn->Define([](Bot* const self) -> Status {
			return SetGoal<node::GoalKind::Terrorist_Spawn>(self);
		});

		set_goal_ctspawn->Define(
			[](Bot* const self) -> Status {
			return SetGoal<node::GoalKind::CT_Spawn>(self);
		});

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
					node::czworld.FindPath(&self->routes, self->Origin(), node::czworld.GetOrigin(Goal_Node_ID), self->JoinedTeam());
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
				} else {
					self->PressKey(ActionKey::Run);
				}
				return Status::Running;
			}
			return Status::Failed;
#endif
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
}