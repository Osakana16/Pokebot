module pokebot.bot.behavior: behavior_definitions;
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
import pokebot.terrain.graph;
import pokebot.terrain.graph.node;
import pokebot.terrain.goal;
import pokebot.util;

namespace pokebot::bot::behavior {
	void DefineAction() {
		auto LookAt = [](Bot* const self, const game::Game* const game, const node::Graph* const graph, const Vector& Dest, const float Range) POKEBOT_NOEXCEPT->Status{
			self->look_direction.view = Dest;
			self->look_direction.movement = Dest;
			return Status::Success;
		};

		reset_goal->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (self->goal_node == node::Invalid_NodeID && self->next_dest_node == node::Invalid_NodeID)
				return Status::Failed;

			self->goal_queue.Clear();
			self->next_dest_node = self->goal_node = node::Invalid_NodeID;
			self->routes.Clear();
			return Status::Success;
		});

		change_primary->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (self->HasPrimaryWeapon()) {
				self->SelectPrimaryWeapon();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		change_secondary->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
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

		look_c4->Define([LookAt](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			return LookAt(self, game, graph, *game->GetDemolitionManager()->GetC4Origin() - Vector{ 0, 0, 36 }, 1.0f);
		});

		look_hostage->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		look_enemy->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (self->HasEnemy()) {
				self->LookAtClosestEnemy();
				return Status::Success;
			} else {
				return Status::Failed;
			}
		});

		look_door->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		look_button->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
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

		set_goal_team_objective->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
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

		rapid_fire->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (self->IsPressingKey(game::player::ActionKey::Attack)) {
				self->PressKey(game::player::ActionKey::Attack);
				return Status::Running;
			} else {
				self->PressKey(game::player::ActionKey::Attack);
				return Status::Success;
			}
		});

		lock->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			self->LockByBomb();
			return Status::Success;
		});

		set_goal_c4_node->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			auto area = graph->GetNearest(*game->GetDemolitionManager()->GetC4Origin());
			for (const auto& Another_Origin : { Vector{}, Vector{50.0f, 0.0f, 0.0f}, Vector{ -50.0f, 0.0f, 0.0f }, Vector{0.0f, 50.0f, 0.0f}, Vector{0.0f, -50.0f, 0.0f} }) {
				if ((area = graph->GetNearest(*game->GetDemolitionManager()->GetC4Origin() + Another_Origin)) != nullptr) {
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

		move_vector->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (!self->goal_vector.has_value())
				return Status::Failed;

			if (game::Distance(self->Origin(), *self->goal_vector) >= 5.0f) {
				self->look_direction.view = *self->goal_vector;
				self->look_direction.movement = *self->goal_vector;
				self->PressKey(game::player::ActionKey::Run);
			}
			return Status::Success;
		});

		set_goal_c4_vector->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (!game->GetDemolitionManager()->GetC4Origin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *game->GetDemolitionManager()->GetC4Origin();
			return Status::Success;
		});

		set_goal_hostage_vector->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			return Status::Success;
		});

		set_goal_backpack_node->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			auto area = graph->GetNearest(*game->GetDemolitionManager()->GetBackpackOrigin());
			if (area == nullptr) {
				return Status::Failed;
			}
			node::NodeID id = graph->GetNearest(*game->GetDemolitionManager()->GetBackpackOrigin())->m_id;
			if (graph->IsOnNode(self->Origin(), id))
				return Status::Failed;

			if (id != node::Invalid_NodeID && !graph->IsOnNode(self->Origin(), id) && self->goal_queue.AddGoalQueue(id, 1)) {
				return Status::Success;
			} else
				return Status::Failed;
		});

		set_goal_backpack_vector->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			if (!game->GetDemolitionManager()->GetBackpackOrigin().has_value() || self->goal_vector.has_value())
				return Status::Failed;

			self->goal_vector = *game->GetDemolitionManager()->GetBackpackOrigin();
			return Status::Success;
		});

		set_goal_from_team_objective_within_range->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
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

		set_goal_from_c4_within_range->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			auto findCircleLine = [&](const Vector& Origin, const float Distance) POKEBOT_NOEXCEPT->node::NodeID{
				node::NodeID id = node::Invalid_NodeID;
				for (const auto& Line : { Vector(Distance, .0f, .0f), Vector(-Distance, .0f, .0f), Vector(.0f, Distance, .0f), Vector(.0f, -Distance, .0f) }) {
					auto area = graph->GetNearest(*game->GetDemolitionManager()->GetC4Origin() + Line, FLT_MAX);
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

		set_goal_hostage_node->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
			return Status::Failed;
		});

		set_goal_bombspot->Define(BotSetsGoal<node::GoalKind::Bombspot>);
		set_goal_rescuezone->Define(BotSetsGoal<node::GoalKind::Rescue_Zone>);
		set_goal_escapezone->Define(BotSetsGoal<node::GoalKind::Escape_Zone>);
		set_goal_vipsafety->Define(BotSetsGoal<node::GoalKind::Vip_Safety>);
		set_goal_tspawn->Define(BotSetsGoal<node::GoalKind::Terrorist_Spawn>);
		set_goal_ctspawn->Define(BotSetsGoal<node::GoalKind::CT_Spawn>);

		find_goal->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
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

		head_to_goal->Define([](Bot* const self, const game::Game* const game, const node::Graph* const graph) -> Status {
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
}