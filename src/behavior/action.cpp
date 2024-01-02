#include "behavior.hpp"

namespace pokebot::bot::behavior {
	namespace {
		template<node::GoalKind kind>
		Status SetGoal(Bot *const self) noexcept {
			if (node::world.IsOnNode(self->Origin(), self->goal_node))
				return Status::Enough;

			node::NodeID id = node::Invalid_NodeID;
			auto goals = node::world.GetGoal(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				id = goal->second;
				break;
			}

			if (id != node::Invalid_NodeID) {
				self->goal_queue.AddGoalQueue(id, 1);
				return Status::Executed;
			} else
				return Status::Failed;
		}
	}

	auto LookAt = [](Bot* const self, const Vector& Dest) noexcept -> Status {
		if (self->IsLookingAt(Dest))
			return Status::Enough;
		else {
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
		change_primary->Define(
			[](Bot* const self) -> Status {
				Status result = Status::Not_Ready;
				if (self->HasPrimaryWeapon()) {
					game::game.IssueCommand(*game::game.clients.Get(self->Name().data()), "slot1");
					result = Status::Executed;
				}
				return result;
			}
		);

		change_secondary->Define(
			[](Bot* const self) -> Status {
				Status result = Status::Not_Ready;
				if (self->HasSecondaryWeapon()) {
					game::game.IssueCommand(*game::game.clients.Get(self->Name().data()), "slot2");
					result = Status::Executed;
				}
				return result;
			}
		);

		change_melee->Define(
			[](Bot* const self) -> Status {
				self->SelectWeapon(game::Weapon::Knife);
				return Status::Executed;
			}
		);

		change_grenade->Define(
			[](Bot* const self) -> Status {
				self->SelectWeapon(game::Weapon::HEGrenade);
				return Status::Executed;
			}
		);

		change_flashbang->Define(
			[](Bot* const self) -> Status {
				self->SelectWeapon(game::Weapon::Flashbang);
				return Status::Executed;
			}
		);

		change_smoke->Define(
			[](Bot* const self) -> Status {
				self->SelectWeapon(game::Weapon::Smoke);
				return Status::Executed;
			}
		);

		change_c4->Define(
			[](Bot* const self) -> Status {
				self->SelectWeapon(game::Weapon::C4);
				return Status::Executed;
			}
		);

		look_c4->Define(
			[](Bot* const self) -> Status {
				return LookAt(self, manager.C4_Origin);
			}
		);

		look_hostage->Define(
			[](Bot* const self) -> Status {
				return Status::Executed;
			}
		);

		look_enemy->Define(
			[](Bot* const self) -> Status {
				self->LookAtClosestEnemy();
				return Status::Executed;
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

		use->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Use);
				return Status::Executed;
			}
		);

		fire->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Attack);
				return Status::Executed;
			}
		);

		jump->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Jump);
				return Status::Executed;
			}
		);

		duck->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Duck);
				return Status::Executed;
			}
		);

		walk->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Shift);
				return Status::Executed;
			}
		);

		change_silencer->Define(
			[](Bot* const self) -> Status {
				self->PressKey(bot::ActionKey::Attack2);
				return Status::Executed;
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
				node::NodeID id = node::world.GetNearest(manager.C4_Origin);
				if (id != node::Invalid_NodeID) {
					self->goal_queue.AddGoalQueue(id, 1);
					return Status::Executed;
				} else
					return Status::Failed;
			}
		);

		set_goal_hostage->Define(
			[](Bot* const self) -> Status {
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
				return SetGoal<node::GoalKind::Esacpe_Zone>(self);
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
				if (self->goal_node == node::Invalid_NodeID && self->goal_queue.IsEmpty())
					return Status::Not_Ready;
				if (node::world.IsOnNode(self->Origin(), self->goal_node)) {
					return Status::Enough;
				}

				if (!self->goal_queue.IsEmpty()) {
					self->goal_node = self->goal_queue.Get();
				}

				if (self->routes.Empty() || self->routes.IsEnd()) {
					// Find path.

					const auto Goal_Node_ID = self->goal_node;
					const auto Current_Node_ID = node::world.GetNearest(self->Origin());
					if (Current_Node_ID == Goal_Node_ID) {
						return Status::Executed;
					} else if (Current_Node_ID == node::Invalid_NodeID) {
						return Status::Failed;
					}

					node::world.FindPath(&self->routes, Current_Node_ID, Goal_Node_ID);
					if (self->routes.Empty()) {
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
			}
		);

		discard_latest_goal->Define
		(
			[](Bot* const self) -> Status {
				if (self->goal_queue.IsEmpty())
					return Status::Enough;

				self->goal_queue.Pop();
				return Status::Executed;
			}
		);

		follow_squad_leader->Define
		([](Bot* const self) -> Status {
			auto leader = manager.GetSquadLeader(self->JoinedTeam(), self->squad);
			self->goal_queue.AddGoalQueue(node::world.GetNearest(leader->origin), 5);
			return Status::Executed;
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
}