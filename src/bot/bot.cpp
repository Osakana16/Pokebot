module pokebot.bot: player_ai;
import pokebot.bot.behavior;

import std;
import pokebot.engine;
import pokebot.game;
import pokebot.game.client;
import pokebot.game.player;
import pokebot.game.util;
import pokebot.util;
import pokebot.util.tracer;

namespace pokebot::bot {
	Bot::Bot(pokebot::node::Graph& graph_, const std::string_view& Bot_Name, const game::Team Join_Team, const game::Model Select_Model) POKEBOT_NOEXCEPT : graph(graph_) {
		team = Join_Team;
		model = Select_Model;
		name = Bot_Name.data();

		OnNewRound();
	}

	void Bot::Run() POKEBOT_NOEXCEPT {
		(this->*(updateFuncs[static_cast<int>(start_action)]))();
		frame_interval = gpGlobals->time - last_command_time;

		const std::uint8_t Msec_Value = ComputeMsec();
		last_command_time = gpGlobals->time;

		pokebot::game::client::Client* client = const_cast<pokebot::game::client::Client*>(game::game.clients.Get(Name().data()));
		client->flags |= pokebot::util::Third_Party_Bot_Flag;
		g_engfuncs.pfnRunPlayerMove(client->Edict(),
				movement_angle,
				move_speed,
				strafe_speed,
				0.0f,
				client->button,
				client->impulse,
				Msec_Value);

		move_speed = strafe_speed = 0.0f;
		UnlockByBomb();
	}

	void Bot::TurnViewAngle() {
		auto client = game::game.clients.Get(Name().data());
		assert(client != nullptr);
		Vector destination{};
		game::OriginToAngle(&destination, *look_direction.view, Origin());
		if (destination.x > 180.0f) {
			destination.x -= 360.0f;
		}

		auto v_angle = client->v_angle;
		v_angle.x = destination.x;

		if (std::abs(destination.y - v_angle.y) <= 1.0f) {
			v_angle.y = destination.y;
		} else {
			auto AngleClamp = [](const float angle, const float destination) {
				return (angle > destination ? std::clamp(angle, destination, angle) : std::clamp(angle, angle, destination));
			};

			auto CalculateNextAngle = [](const float dest, const float angle) POKEBOT_NOEXCEPT{
				return std::clamp(dest - angle, -180.0f, 180.0f);
			};

			constexpr float Base_Frame = 30.0f;
			constexpr float Sensitivity = 1.0f;
			const Vector Next_Angle = {
				CalculateNextAngle(destination.x, v_angle.x) / (Base_Frame - Sensitivity),
				CalculateNextAngle(destination.y, v_angle.y) / (Base_Frame - Sensitivity),
				0.0
			};

			v_angle.y += Next_Angle.y;
			v_angle.y = AngleClamp(v_angle.y, destination.y);
		}
		v_angle.z = 0.0f;
		client->v_angle = v_angle;
		client->angles.x = client->v_angle.x / 3;
		client->angles.y = client->v_angle.y;
		client->v_angle.x = -client->v_angle.x;
		client->angles.z = 0;
			
		client->ideal_yaw = client->v_angle.y;
		if (client->ideal_yaw > 180.0f) {
			client->ideal_yaw -= 360.0f;
		} else if (client->ideal_yaw < -180.0f) {
			client->ideal_yaw += 360.0f;
		}
			
		client->idealpitch = client->v_angle.x;
		if (client->idealpitch > 180.0f) {
			client->idealpitch-= 360.0f;
		} else if (client->idealpitch < -180.0f) {
			client->idealpitch += 360.0f;
		}
	}

	void Bot::TurnMovementAngle() {
		game::OriginToAngle(&movement_angle, *look_direction.movement, Origin());
	}

	void Bot::OnNewRound() POKEBOT_NOEXCEPT {
		goal_queue.Clear();
		routes.Clear();

		goal_node = node::Invalid_NodeID;
		next_dest_node = node::Invalid_NodeID;

		look_direction.Clear();
		ideal_direction.Clear();

		danger_time.SetTime(0);
		freeze_time.SetTime(g_engfuncs.pfnCVarGetFloat("mp_freezetime") + 1.0f);
		spawn_cooldown_time.SetTime(1.0f);

		start_action = Message::Buy;
		buy_wait_timer.SetTime(1.0f);

		// mapdata::BSP().LoadWorldMap();
	}

	void Bot::NormalUpdate() POKEBOT_NOEXCEPT {
		auto client = game::game.clients.GetAsMutable(Name().data());
		assert(client != nullptr);
		assert(JoinedTeam() != game::Team::Spector && JoinedTeam() != game::Team::Random);
		if (client->IsDead()) {
			return;
		}

		if (!freeze_time.IsRunning() && !spawn_cooldown_time.IsRunning()) {
			// Do not do anything when freezing.
			// Due to game engine specifications or bugs, 
			// if we execute a heavy process immediately after the start of a round, 
			// the game will freeze.
#if 0
			// NOTE: This code makes the game laggy so it is temporary disabled.
			// 
			// If the bot is not in the route, reset it.
			if (auto current_id = node::czworld.GetNearest(Origin()); current_id != nullptr) {
				if (!routes.Contains(current_id->m_id)) {
					routes.Clear();
				}
			}
#endif
			(this->*(doObjective[static_cast<int>(state)]))();

			// - Bot Navigation -

			CheckAround();		// Update the entity's viewment.
			CheckBlocking();	// Check the something is blocking myself.

			if (IsJumping()) {
				PressKey(game::player::ActionKey::Duck);
			}

			// Move forward if the bot has the route.
			if (!routes.Empty() && next_dest_node != node::Invalid_NodeID) {
#if 0
				if (!stuck_check_interval_timer.IsRunning()) {
					if (common::Distance(Origin(), stuck_check_origin) <= 5.0f) {
						// Stuck detected.
						routes.Clear();
					}
					stuck_check_origin = Origin();
					stuck_check_interval_timer.SetTime(1.0f);
				}
#endif
				if (look_direction.view.has_value() && look_direction.movement.has_value()) {
					PressKey(game::player::ActionKey::Run);
				}
			}
			look_direction.Clear();

			// - Update Key -
			
			constexpr float Default_Max_Move_Speed = 255.0f;
			if (bool(press_key & game::player::ActionKey::Run)) {
				if (bool(press_key & game::player::ActionKey::Shift)) {
					press_key &= ~game::player::ActionKey::Shift;
					move_speed = game::Default_Max_Move_Speed / 2.0f;
				} else {
					move_speed = game::Default_Max_Move_Speed;
				}
			}

			if (bool(press_key & game::player::ActionKey::Move_Right)) {
				strafe_speed = game::Default_Max_Move_Speed;
			}

			if (bool(press_key & game::player::ActionKey::Move_Left)) {
				strafe_speed = -game::Default_Max_Move_Speed;
			}

			if (bool(press_key & game::player::ActionKey::Back)) {

			}

			if (bool(press_key & game::player::ActionKey::Attack)) {

			}

			if (bool(press_key & game::player::ActionKey::Attack2)) {

			}

			if (bool(press_key & game::player::ActionKey::Use)) {

			}

			if (bool(press_key & game::player::ActionKey::Jump)) {
				if (!client->IsOnFloor()) {
					return;
				}
			}

			if (bool(press_key & game::player::ActionKey::Duck)) {

			}

			client->PressKey(static_cast<int>(press_key));
			press_key = game::player::ActionKey::None;
		} else {
			// While freezing.
			stopping_time = gpGlobals->time + 5.0f;
		}
	}

	void Bot::OnSelectionCompleted() POKEBOT_NOEXCEPT {
		start_action = Message::Buy;
	}

	void Bot::AccomplishMission() POKEBOT_NOEXCEPT {
		auto current_mode = game::game.GetScenario();
		(this->*(accomplishState[static_cast<int>(JoinedTeam()) - 1][static_cast<int>(std::log2(static_cast<float>(current_mode)))]))();
	}

	void Bot::OnTerroristDemolition() noexcept {
#if 0
		if (game::game.GetC4Origin().has_value()) {
			behavior::demolition::t_planted_wary->Evaluate(this);
		} else if (game::game.GetBackpackOrigin().has_value()) {
			behavior::demolition::t_pick_up_bomb->Evaluate(this);
		} else {
			if (HasWeapon(game::weapon::ID::C4)) {
				behavior::demolition::t_plant->Evaluate(this);
			}
		}
#endif
	}

	void Bot::OnTerroristHostage() noexcept {
		behavior::rescue::t_defend_hostage->Evaluate(this, &graph);
	}

	void Bot::OnTerroristAssasination() noexcept {
		auto client = game::game.clients.Get(Name().data());
		if (client->IsVIP()) {
			behavior::assassination::ct_vip_escape->Evaluate(this, &graph);
		} else {

		}
	}

	void Bot::OnTerroristEscape() noexcept {

	}

	void Bot::OnCTDemolition() noexcept {
#if 0
#endif
	}

	void Bot::OnCTHostage() noexcept {
		auto client = game::game.clients.Get(Name().data());
		if (!client->HasHostages()) {
			behavior::rescue::ct_try->Evaluate(this, &graph);
		} else {
			behavior::rescue::ct_leave->Evaluate(this, &graph);
		}
	}

	void Bot::OnCTAssasination() noexcept {
		auto client = game::game.clients.Get(Name().data());
		if (client->IsVIP()) {
			behavior::assassination::ct_vip_escape->Evaluate(this, &graph);
		} else {

		}
	}

	void Bot::OnCTEscape() noexcept {

	}

	void Bot::Combat() POKEBOT_NOEXCEPT {
		auto client = game::game.clients.Get(Name().data());
		assert(!target_enemies.empty() && "The bot has no enemies despite being in combat mode.");
		/*
			Choose to fight or to flee.
			The bot has 'fighting spirit' that affects willingness to fight.
		*/

		constexpr int Max_Health = 100;
		constexpr int Max_Armor = 100;
		constexpr int Base_Fearless = Max_Health / 2 + Max_Armor / 5;

		const pokebot::game::client::Client *enemy_client{};
		for (auto& target_name : target_enemies) {
			auto target_client = game::game.clients.Get(target_name.c_str());
			if (!target_client->IsDead()) {
				enemy_client = target_client;
				break;
			}
		}
		if (enemy_client == nullptr) {
			danger_time.SetTime(0.0f);
			return;
		}

		const bool Enemy_Has_Primary = enemy_client->HasPrimaryWeapon();
		const bool Enemy_Has_Secondary = enemy_client->HasSecondaryWeapon();

		int fighting_spirit = Max_Health + Max_Armor + (HasPrimaryWeapon() ? 50 : 0) + (HasSecondaryWeapon() ? 50 : 0);
		if (!HasPrimaryWeapon() && !HasSecondaryWeapon() ||
			(enemy_client->IsOutOfClip() && enemy_client->IsOutOfCurrentWeaponAmmo())) {
			// If I have no guns.
			if (Enemy_Has_Primary || Enemy_Has_Secondary) {
				// The enemy has weapon so I should flee.
				fighting_spirit = std::numeric_limits<decltype(fighting_spirit)>::min();
			} else {
				// This is the good chance to beat enemies.
			}
		} else {
			if (IsReloading() || enemy_client->IsOutOfClip()) {
				// I'm reloading so I have to flee.
				fighting_spirit = std::numeric_limits<decltype(fighting_spirit)>::min();
			} else {
				// If I have guns.
				fighting_spirit -= (Max_Health - Health());
				fighting_spirit -= (Max_Armor - enemy_client->armor);
				// fighting_spirit -= (Enemy_Has_Primary ? 25 : 0);
				// fighting_spirit -= (Enemy_Has_Secondary ? 25 : 0);
			}
		}

		if (fighting_spirit <= Base_Fearless) {
			behavior::fight::retreat->Evaluate(this, &graph);
		}

		if (CanSeeEntity(enemy_client->Edict())) {
			behavior::fight::beat_enemies->Evaluate(this, &graph);
		}
	}

	void Bot::Follow() POKEBOT_NOEXCEPT {
#if 0
		const auto Leader_Client = Manager::Instance().GetLeader(JoinedTeam(), JoinedPlatoon());
		assert(Leader_Client != nullptr);
		const auto Leader_Origin = Leader_Client->origin;
		if (const auto Leader_Area = node::czworld.GetNearest(Leader_Origin); Leader_Area != nullptr) {
			if (goal_node == node::Invalid_NodeID) {
				goal_node = Leader_Area->m_id;
			}

			if (!routes.Empty() && !routes.IsEnd()) {
				if (const auto Current_Area = node::czworld.GetNearest(Origin()); Current_Area != nullptr && Current_Area != Leader_Area) {
					// - Check -
					const auto Current_Node_ID = Current_Area->m_id;
					if (goal_node == Current_Node_ID) {
						// The bot is already reached at the destination.
						goal_queue.Clear();
						routes.Clear();
						goal_node = node::Invalid_NodeID;
					} else {
						if (next_dest_node == node::Invalid_NodeID) {
							next_dest_node = routes.Current();
						} else if (node::czworld.IsOnNode(Origin(), next_dest_node)) {
							if (!routes.IsEnd()) {
								if (routes.Next()) {
									next_dest_node = routes.Current();
								}
							}
						}
					}
				}
			} else {
				auto origin = node::czworld.GetOrigin(goal_node);
				node::czworld.FindPath(&routes, Origin(), *reinterpret_cast<Vector*>(&*origin), JoinedTeam());
				if (routes.Empty()) {
					goal_node = node::Invalid_NodeID;
				}
			}

			const auto Ditance_To_Leader = game::Distance(Origin(), Leader_Origin);
			if (Ditance_To_Leader <= 275.0f) {
				if (Ditance_To_Leader >= 75.0f) {
					if (Leader_Client->IsDucking()) {
						PressKey(game::player::ActionKey::Duck);
					} else if (Leader_Client->IsWalking()) {
						PressKey(game::player::ActionKey::Run | game::player::ActionKey::Shift);
					} else {
						PressKey(game::player::ActionKey::Run);
					}
				}
			} else {
				PressKey(game::player::ActionKey::Run);
			}
		}
#endif
	}

	void Bot::TryToUnstuck() {
		/* 
		* The function tries to unstuck by some actions; jump or reset the goal.
		* 
		* Bots get stuck due to various causes:
		*	1. Collide the wall.
		*	2. Collide the other players.
		*	3. Stop by unknown reasons.
		*/
#if 1
		goal_queue.Clear();
		goal_vector = std::nullopt;
		goal_node = node::Invalid_NodeID;

		stopping_time = gpGlobals->time + 5.0f;
#endif
		state = State::Accomplishment;
	}

	template<typename Array>
	std::map<float, int> SortedDistances(const Vector& Base, const Array& list) {
		std::map<float, int> result{};
		for (int i = 0; i < list.size(); i++) {
			result[game::Distance(Base, game::game.clients.Get(list[i].c_str())->origin)] = i;
		}
		return result;
	}

	void Bot::CheckAround() {
		auto next_origin = graph.GetOrigin(next_dest_node);
		if (!look_direction.view.has_value()) {
			look_direction.view = *reinterpret_cast<Vector*>(&next_origin);
			look_direction.view->z = Origin().z;
		}

		if (!look_direction.movement.has_value()) {
			look_direction.movement = *reinterpret_cast<Vector*>(&next_origin);
			look_direction.movement->z = Origin().z;
		}


		TurnViewAngle();
		TurnMovementAngle();
#if 1
		if (game::poke_fight) {
			auto client = game::game.clients.Get(Name().data());
			pokebot::util::PlayerName enemies_in_view[32]{};
			int i{};

			client->GetEnemyNamesWithinView(enemies_in_view);
			if (!enemies_in_view[0].empty()) {
				for (auto& enemy : enemies_in_view) {
					if (enemy.empty()) {
						break;
					}
					if (std::find(target_enemies.begin(), target_enemies.end(), enemy) == target_enemies.end()) {
						target_enemies.push_back(enemy);
					}
				}
				danger_time.SetTime(5.0);
			}
		}
#endif
		if (!danger_time.IsRunning()) {
			target_enemies.clear();
			state = State::Accomplishment;
		} else {
			state = State::Crisis;
		}
	}

	void Bot::CheckBlocking() noexcept {
		if (is_locked_by_bomb)
			return;

		// Check if the player is blocking and avoid it.
		for (edict_t* entity{}; (entity = game::FindEntityInSphere(entity, Origin(), 90.0f)) != nullptr;) {
			if (std::string_view(STRING(entity->v.classname)) == "player") {
				if (CanSeeEntity(entity)) {
					PressKey(game::player::ActionKey::Move_Left);
					return;
				}
			}
		}

		// Check if the worldspawn is blocking:
		const auto Foot = Origin();
		const auto Knee = Origin() + Vector(.0f, .0f, 36.0f);
		const auto Head = Foot + game::game.clients.Get(Name().data())->view_ofs;

		auto CheckHead = [&] () -> bool {
			util::Tracer tracer{};
			const bool Is_Head_Forward_Center_Hit = tracer.MoveStart(Head).MoveDest(Head + gpGlobals->v_forward * 90.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit();
			const bool Is_Head_Forward_Left_Hit = tracer.MoveDest(Head + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * -45.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit();
			const bool Is_Head_Forward_Right_Hit = tracer.MoveDest(Head + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * 45.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit();
			const bool Is_Head_Forward_Hit = Is_Head_Forward_Center_Hit || Is_Head_Forward_Left_Hit || Is_Head_Forward_Right_Hit;

			if (Is_Head_Forward_Hit) {
				// Check left
				tracer.MoveDest(Head + gpGlobals->v_right * -90.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr);
				if (tracer.IsHit()) {
					PressKey(game::player::ActionKey::Move_Right);
				} else {
					// Check right
					tracer.MoveDest(Head + gpGlobals->v_right * 90.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr);
					if (tracer.IsHit()) {
						PressKey(game::player::ActionKey::Move_Left);
					}
				}
				return true;
			}
			return false;
		};

		auto CheckBody = [&] () {
			util::Tracer tracer{};

			const bool Is_Knee_Forward_Center_Hit = tracer.MoveStart(Foot).MoveDest(Knee + gpGlobals->v_forward * 90.0f).TraceHull(util::Tracer::Monsters::Ignore, util::Tracer::HullType::Head, nullptr).IsHit();
			const bool Is_Foot_Forward_Center_Hit = tracer.MoveStart(Foot).MoveDest(Foot + gpGlobals->v_forward * 90.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit();
			const bool Is_Foot_Forward_Left_Hit = tracer.MoveDest(Foot + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * -45.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit();
			const bool Is_Foot_Forward_Right_Hit = tracer.MoveDest(Foot + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * 45.0f).TraceLine(util::Tracer::Monsters::Ignore, nullptr).IsHit();
			const bool Is_Foot_Forward_Hit = Is_Foot_Forward_Center_Hit || Is_Foot_Forward_Left_Hit || Is_Foot_Forward_Right_Hit;
			if (Is_Knee_Forward_Center_Hit) {
				PressKey(game::player::ActionKey::Jump);
			} else {
				// Check the forward navarea
				for (auto& vector : { Origin(), Origin() + gpGlobals->v_forward * 90.0f }) {
					// 
					if (auto area = graph.GetNearest(vector); area != nullptr) {
						// Jump if it is specified.
						if (!graph.HasFlag(area->m_id, node::NavmeshFlag::No_Jump) && graph.HasFlag(area->m_id, node::NavmeshFlag::Jump)) {
							PressKey(game::player::ActionKey::Jump);
						}
						// Duck if it is specified.
						if (graph.HasFlag(area->m_id, node::NavmeshFlag::Crouch)) {
							PressKey(game::player::ActionKey::Duck);
						}
					}
				}
			}
		};
		
		if (!CheckHead()) {
			CheckBody();
		}

		if (!game::game.clients.Get(Name().c_str())->IsStopping()) {
			stopping_time = gpGlobals->time + 1.0f;
		}

		if (stopping_time < gpGlobals->time) {
			state = State::Stuck;
		}
	}

	bool Bot::CanSeeEntity(const edict_t* entity) const POKEBOT_NOEXCEPT {
		auto client = game::game.clients.Get(Name().data()); client->CanSeeEntity(entity);
		return client->CanSeeEntity(entity);
	}
	
	bool Bot::IsPressingKey(const game::player::ActionKey Key) const noexcept {
		auto client = game::game.clients.Get(Name().data());
		return (client->button & static_cast<int>(Key));
	}

	void Bot::SelectionUpdate() POKEBOT_NOEXCEPT {
		int value{};
		switch (start_action) {
			case Message::Team_Select:
			{
				// assert(JoinedTeam() != common::Team::T && JoinedTeam()  != common::Team::CT);
				value = static_cast<int>(JoinedTeam());
				break;
			}
			case Message::Model_Select:
			{
				assert(JoinedTeam() != game::Team::Spector && JoinedTeam() != game::Team::Random);
				start_action = Message::Selection_Completed;
				value = static_cast<int>(model);
				break;
			}
			default:
			{
				assert(false);
				return;
			}
		}
		auto client = game::game.clients.Get(Name().data());
		engine::EngineInterface::InputFakeclientCommand(client->client, std::format("menuselect {}", value).c_str());
	}

	void Bot::SelectWeapon(const game::weapon::ID Target_Weapon) {
		if (HasWeapon(Target_Weapon)) {
			current_weapon = Target_Weapon;
			auto client = game::game.clients.Get(Name().data());
			engine::EngineInterface::InputFakeclientCommand(client->client, std::format("{}", std::get<game::weapon::WeaponName>(game::weapon::Weapon_CVT[static_cast<int>(Target_Weapon) - 1])).c_str());
		}
	}

	void Bot::SelectPrimaryWeapon() {
		SelectWeapon(static_cast<game::weapon::ID>(std::log2(game::game.clients.Get(Name().data())->weapons & game::weapon::Primary_Weapon_Bit)));
	}

	void Bot::SelectSecondaryWeapon() {
		SelectWeapon(static_cast<game::weapon::ID>(std::log2(game::game.clients.Get(Name().data())->weapons & game::weapon::Secondary_Weapon_Bit)));
	}

	void Bot::LookAtClosestEnemy() {
		if (target_enemies.empty()) {
			return;
		}
		const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
		const auto& Nearest_Enemy = game::game.clients.Get(target_enemies[Enemy_Distances.begin()->second].c_str());
		look_direction.view = Nearest_Enemy->origin - Vector(20.0f, 0, 0);
	}

	bool Bot::HasEnemy() const POKEBOT_NOEXCEPT {
		return !target_enemies.empty();
	}

	bool Bot::IsLookingAtEnemy() const POKEBOT_NOEXCEPT {
		if (target_enemies.empty()) {
			return false;
		}

		const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
		const auto& Nearest_Enemy = game::game.clients.Get(target_enemies[Enemy_Distances.begin()->second].c_str());
		return IsLookingAt(Nearest_Enemy->origin, 1.0f);
	}

	bool Bot::IsEnemyFar() const POKEBOT_NOEXCEPT {
		if (target_enemies.empty()) {
			return false;
		}
		const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
		const auto& Nearest_Enemy = game::game.clients.Get(target_enemies[Enemy_Distances.begin()->second].c_str());
		return game::Distance(Origin(), Nearest_Enemy->origin) > 1000.0f;
	}

	bool Bot::IsLookingAt(const Vector& Dest, const float Range) const POKEBOT_NOEXCEPT {
		float vecout[3]{};
		Vector angle = Dest - Origin();
		VEC_TO_ANGLES(angle, vecout);
		if (vecout[0] > 180.0f)
			vecout[0] -= 360.0f;
		vecout[0] = -vecout[0];
		auto result = game::Distance2D(Vector(vecout), game::game.clients.Get(Name().data())->v_angle);
		return (result <= Range);
	}

	bool Bot::HasGoalToHead() const POKEBOT_NOEXCEPT {
		return goal_node != node::Invalid_NodeID;
	}


	bool Bot::IsInBuyzone() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->IsInBuyzone(); }

	const pokebot::util::PlayerName& Bot::Name() const POKEBOT_NOEXCEPT { return name; }

	Vector Bot::Origin() const POKEBOT_NOEXCEPT {
		return game::game.clients.Get(Name().data())->origin;
	}

	float Bot::Health() const POKEBOT_NOEXCEPT {
		return game::game.clients.Get(Name().data())->health;
	}

	bool Bot::HasPrimaryWeapon() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->HasPrimaryWeapon()); }
	bool Bot::HasSecondaryWeapon() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->HasSecondaryWeapon()); }
	bool Bot::HasWeapon(const game::weapon::ID Weapon_ID) const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->HasWeapon(Weapon_ID); }
	bool Bot::IsDucking() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsDucking()); }
	bool Bot::IsDriving() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsOnTrain()); }
	bool Bot::IsInWater() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsInWater()); }
	bool Bot::IsSwimming() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsInWater()); }
	bool Bot::IsOnFloor() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsOnFloor()); }
	bool Bot::IsClimbingLadder() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->IsClimblingLadder(); }
	bool Bot::IsReloading() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->IsPlayerModelReloading(); }
	bool Bot::IsPlantingBomb() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsChangingWeapon() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsFalling() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::Jumped() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsJumping() const POKEBOT_NOEXCEPT { return !game::game.clients.Get(Name().data())->IsOnFloor(); }
	bool Bot::IsLeadingHostages() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->HasHostages(); }
	bool Bot::IsLookingThroughScope() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsLookingThroughCamera() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsChangingSilencer() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsEnabledFlashlight() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsEnabledNightvision() const POKEBOT_NOEXCEPT { return false; }

	uint8_t Bot::ComputeMsec() POKEBOT_NOEXCEPT {
		return static_cast<std::uint8_t>(std::min({ static_cast<int>(std::roundf((gpGlobals->time - last_command_time) * 1000.0f)), 255 }));
	}

	void Bot::OnRadioRecieved(const std::string_view& Sender_Name, const std::string_view& Radio_Sentence) POKEBOT_NOEXCEPT {
		static bool is_sent{};
		auto client = game::game.clients.Get(name.c_str());
		auto Ignore = [] { /* Do nothing */ };
		auto AlwaysPositive = [&] { engine::EngineInterface::InputFakeclientCommand(client->client, "radio3"); engine::EngineInterface::InputFakeclientCommand(client->client, "menuselect 1"); };
		auto AlwaysNegative = [&] { engine::EngineInterface::InputFakeclientCommand(client->client, "radio3"); engine::EngineInterface::InputFakeclientCommand(client->client, "menuselect 8"); };

		const std::unordered_map<util::fixed_string<60u>, std::function<void()>, util::fixed_string<60u>::Hash> Radios{
			{ "#Cover_me", Ignore },
			{ "#You_take_the_point", Ignore },
			{ "#Hold_this_position", Ignore },
			{
				"#Regroup_team",
				[&] {
						goal_queue.AddGoalQueue(graph.GetNearest(Origin())->m_id);
						engine::EngineInterface::InputFakeclientCommand(client->client, "radio3");
						engine::EngineInterface::InputFakeclientCommand(client->client, "menuselect 1");
					}
				},
				{ "#Follow_me", AlwaysPositive },
				{ "#Taking_fire", Ignore },
				{
					"#Go_go_go",
					[&] {
						goal_node = node::Invalid_NodeID;
						goal_vector = std::nullopt;
						goal_queue.Clear();
						engine::EngineInterface::InputFakeclientCommand(client->client, "radio3");
						engine::EngineInterface::InputFakeclientCommand(client->client, "menuselect 1");
					}
				},
				{ "#Team_fall_back", Ignore },
				{ "#Stick_together_team", AlwaysPositive },
				{ "#Get_in_position_and_wait", Ignore },
				{ "#Storm_the_front", Ignore },
				{ "#Report_in_team", [] {} },
				{ "#Affirmative", Ignore },
				{ "#Roger_that", Ignore },
				{ "#Enemy_spotted", Ignore },
				{ "#Need_backup", Ignore },
				{ "#Sector_clear", Ignore },
				{ "#In_position", Ignore },
				{"#Reporting_in", Ignore },
				{ "#Get_out_of_there", Ignore },
				{ "#Negative", Ignore },
				{ "#Enemy_down", Ignore },
				{ "#Fire_in_the_hole", Ignore },
				// Ex-Radio
				{ "Team, infiltrate and eliminate.", Ignore },
				{ "Focus efforts on the bombsite closest to CT spawn.", Ignore },
				{ "Focus efforts on the bombsite furthest from CT spawn.", Ignore },
				{ "Protect bomber.", Ignore },
				{ "Take the rescue zones.", Ignore },
				{ "Cover the V.I.P.", Ignore },
				{ "Take the safety zones.", Ignore },
				{ "Take the escape zones.", Ignore },
				{ "All sticks together.", Ignore },
				{ "Split up some squads.", Ignore },
		};
		Radios.at(Radio_Sentence.data())();
	}


	void Bot::OnBombPlanted() POKEBOT_NOEXCEPT {
		switch (JoinedTeam()) {
			case game::Team::CT:
				goal_queue.Clear();
				break;
		}
	}

	game::Team Bot::JoinedTeam() const POKEBOT_NOEXCEPT {
		return team;
	}
}