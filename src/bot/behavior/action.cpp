#include "behavior.hpp"
#include "bot/manager.hpp"
#include <future>

namespace pokebot::bot::behavior {
	namespace {
		template<node::GoalKind kind>
		Status SetGoal(Bot *const self) POKEBOT_NOEXCEPT {
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

	auto LookAt = [](Bot* const self, const Vector& Dest, const float Range) POKEBOT_NOEXCEPT -> Status {
		self->look_direction.view = Dest;
		self->look_direction.movement = Dest;
		return Status::Success;
	};

	
	template<ActionKey key>
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

	template<game::Weapon weapon>
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

		change_melee->Define(BotChangesIfNotSelected<game::Weapon::Knife>);
		change_grenade->Define(BotChangesIfNotSelected<game::Weapon::HEGrenade>);
		change_flashbang->Define(BotChangesIfNotSelected<game::Weapon::Flashbang>);
		change_smoke->Define(BotChangesIfNotSelected<game::Weapon::Smoke>);
		change_c4->Define(BotChangesIfNotSelected<game::Weapon::C4>);

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

		move_forward->Define(BotPressesKey<bot::ActionKey::Run>);
		use->Define(BotPressesKey<bot::ActionKey::Use>);
		tap_fire->Define(BotPressesKey<bot::ActionKey::Attack>);
		jump->Define(BotPressesKey<bot::ActionKey::Jump>);
		duck->Define(BotPressesKey<bot::ActionKey::Duck>);
		walk->Define(BotPressesKey<bot::ActionKey::Shift>);
		change_silencer->Define(BotPressesKey<bot::ActionKey::Attack2>);
		adjust_scope->Define(BotPressesKey<bot::ActionKey::Attack2>);

		set_goal_team_objective->Define([](Bot* const self) -> Status {
			const bot::PlatoonID Joined_Platoon = self->JoinedPlatoon();
			const common::Team Joined_Team = self->JoinedTeam();
			const node::NodeID Node = Manager::Instance().GetGoalNode(Joined_Team, Joined_Platoon);
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
			if (self->IsPressingKey(ActionKey::Attack)) {
				self->PressKey(ActionKey::Attack);
				return Status::Running;
			} else {
				self->PressKey(ActionKey::Attack);
				return Status::Success;
			}
		});

		lock->Define([](Bot* const self) -> Status {
			self->LockByBomb();
			return Status::Success;
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
#endif
		});

		move_vector->Define([](Bot* const self) -> Status {
			if (!self->goal_vector.has_value())
				return Status::Failed;

			if (common::Distance(self->Origin(), *self->goal_vector) >= 5.0f) {
				self->look_direction.view = *self->goal_vector;
				self->look_direction.movement = *self->goal_vector;
				self->PressKey(ActionKey::Run);
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
			if (self->goal_vector.has_value())
				return Status::Failed;

			auto hostage_id = Manager::Instance().GetTroopTargetedHostage(self->JoinedTeam(), self->JoinedPlatoon());
			if (!hostage_id.has_value()) {
				return Status::Failed;
			}
			
			auto origin = game::game.GetHostageOrigin(*hostage_id);
			if (!origin.has_value()) {
				return Status::Failed;
			}
			self->goal_vector = *origin;
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
					auto area = node::czworld.GetNearest(node::czworld.GetOrigin(bot::Manager::Instance().GetGoalNode(self->JoinedTeam(), self->JoinedPlatoon())) + Line, FLT_MAX);
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
			assert(self->JoinedTeam() == common::Team::CT);

			edict_t* entity{};
			float min_distance = std::numeric_limits<float>::max();
			auto hostage_id = Manager::Instance().GetTroopTargetedHostage(self->JoinedTeam(), self->JoinedPlatoon());
			assert(hostage_id.has_value());

			auto origin = game::game.GetHostageOrigin(*hostage_id);
			assert(origin.has_value());

			auto area = node::czworld.GetNearest(*origin);
			if (area == nullptr)
				return Status::Failed;

			auto id = area->m_id;
			if (id != node::Invalid_NodeID && !node::czworld.IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else {
				return Status::Failed;
			}
#endif
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
				static std::unordered_map<util::PlayerName, Timer, util::PlayerName::Hash> timers{};
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