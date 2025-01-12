#include "behavior.hpp"

#include <thread>

namespace pokebot {
	namespace bot {
		void Bot::Run() POKEBOT_NOEXCEPT {
			const static std::unordered_map<Message, std::function<void(Bot&)>> Update_Funcs{
				{ Message::Team_Select, &Bot::SelectionUpdate },
				{ Message::Model_Select, &Bot::SelectionUpdate },
				{ Message::Buy, &Bot::BuyUpdate },
				{ Message::Normal, &Bot::NormalUpdate },
				{ Message::Selection_Completed, &Bot::OnSelectionCompleted }
			};

			auto update = Update_Funcs.at(start_action);
			update(*this);
			frame_interval = gpGlobals->time - last_command_time;

			const std::uint8_t Msec_Value = ComputeMsec();
			last_command_time = gpGlobals->time;
			game::game.RunPlayerMove(Name().data(), movement_angle, move_speed, strafe_speed, 0.0f, Msec_Value, committer);
			committer.Clear();
			move_speed = 0;
		}

#define ENABLE_NEW_TURN_ANGLE 1

		void Bot::TurnViewAngle() {
			auto client_status = game::game.GetClientStatus(Name().data());
			auto destination = look_direction.view->ToAngleVector(Origin());
			if (destination.x > 180.0f) {
				destination.x -= 360.0f;
			}
			auto v_angle = client_status.v_angle();
			v_angle.x = destination.x;

			if (std::abs(destination.y - v_angle.y) <= 1.0f) {
				v_angle.y = destination.y;
			} else {
				auto AngleClamp = [](const float angle, const float destination) {
					return (angle > destination ? std::clamp(angle, destination, angle) : std::clamp(angle, angle, destination));
				};

				auto CalculateNextAngle = [](const float dest, const float angle) POKEBOT_NOEXCEPT {
					return std::clamp(dest - angle, -180.0f, 180.0f);
				};

				constexpr float Base_Frame = 30.0f;
				constexpr float Sensitivity = 1.0f;
				const common::AngleVector Next_Angle = {
					CalculateNextAngle(destination.x, v_angle.x) / (Base_Frame - Sensitivity),
					CalculateNextAngle(destination.y, v_angle.y) / (Base_Frame - Sensitivity),
					0.0
				};

				v_angle.y += Next_Angle.y;
				v_angle.y = AngleClamp(v_angle.y, destination.y);
			}
			v_angle.z = 0.0f;
			committer.TurnViewAngle(v_angle);
		}

		void Bot::TurnMovementAngle() {
			movement_angle = look_direction.movement->ToAngleVector(Origin());
		}

		void Bot::OnNewRound() POKEBOT_NOEXCEPT {
			goal_queue.Clear();
			routes.Clear();

			goal_node = node::Invalid_NodeID;
			next_dest_node = node::Invalid_NodeID;

			LeavePlatoon();
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
			auto client_status = game::game.GetClientStatus(Name().data());
			assert(JoinedTeam() != common::Team::Spector && JoinedTeam() != common::Team::Random);
			if (client_status.IsDead()) {
				return;
			}
			
			if (!freeze_time.IsRunning() && !spawn_cooldown_time.IsRunning()) {
				if (state != State::Crisis && Manager::Instance().IsFollowerPlatoon(JoinedTeam(), JoinedPlatoon())) {
					state = State::Follow;
				}

				// Do not do anything when freezing.
				// Due to game engine specifications or bugs, 
				// if we execute a heavy process immediately after the start of a round, 
				// the game will freeze.

				switch (state) {
					case State::Accomplishment:
						AccomplishMission();
						break;
					case State::Crisis:
						Combat();
						break;
					case State::Follow:
						Follow();
						break;
				}
				CheckAround();

				for (auto& vector : { Vector(0.0f, 0.0f, 0.0f), Vector(50.0f, 0.0f, 0.0f), Vector(-50.0f, 0.0f, 0.0f), Vector(0.0f, 50.0f, 0.0f), Vector(0.0f, -50.0f, 0.0f) }) {
					if (auto area = node::czworld.GetNearest(Origin() + vector); area != nullptr && node::czworld.HasFlag(area->m_id, node::NavmeshFlag::Jump)) {
						PressKey(ActionKey::Jump);
						break;
					}
				}

				if (!routes.Empty() && next_dest_node != node::Invalid_NodeID) {
					if (look_direction.view.has_value() && look_direction.movement.has_value()) {
						auto r = look_direction.movement->ToAngleVector(Origin()) - look_direction.view->ToAngleVector(Origin());
						PressKey(ActionKey::Run);
					}
				}
				look_direction.Clear();
			}
		}

		void Bot::OnSelectionCompleted() POKEBOT_NOEXCEPT {
			Manager::Instance().OnBotJoinedCompletely(this);
			start_action = Message::Buy;
		}

		void Bot::AccomplishMission() POKEBOT_NOEXCEPT {
			static const auto Do_Nothing_If_Mode_Is_Not_Applicable = std::make_pair<game::MapFlags, std::function<void()>>(static_cast<game::MapFlags>(0), [this] { /* Do nothing. */ });
			switch (JoinedTeam()) {
				case common::Team::T:
				{
					std::unordered_map<game::MapFlags, std::function<void()>> modes{
						Do_Nothing_If_Mode_Is_Not_Applicable,
						{
							game::MapFlags::Demolition,
							[&] {
								if (Manager::Instance().C4Origin().has_value()) {
									behavior::demolition::t_planted_wary->Evaluate(this);
								} else {
									if (HasWeapon(game::Weapon::C4)) {
										behavior::demolition::t_plant->Evaluate(this);
									}
								}
							}
						},
						{
							game::MapFlags::HostageRescue,
							[&] {
							
							}
						},
						{
							game::MapFlags::Assassination,
							[&] {
							
							}
						},
						{
							game::MapFlags::Escape,
							[&] {
								behavior::escape::t_take_point->Evaluate(this);
							}
						},
					};
					auto current_mode = game::game.GetMapFlag();
					for (auto supported_mode : modes) {
						modes[current_mode & supported_mode.first]();
					}
					break;
				}
				case common::Team::CT:
				{
					std::unordered_map<game::MapFlags, std::function<void()>> modes{
						Do_Nothing_If_Mode_Is_Not_Applicable,
						{
							game::MapFlags::Demolition,
							[&] {
								if (Manager::Instance().C4Origin().has_value()) {
									if (common::Distance(Origin(), *Manager::Instance().C4Origin()) <= 50.0f) {
										behavior::demolition::ct_defusing->Evaluate(this);
									} else {
										behavior::demolition::ct_planted->Evaluate(this);
									}
								} else {
									behavior::demolition::ct_defend->Evaluate(this);
								}
							}
						},
						{
							game::MapFlags::HostageRescue,
							[&] {
								auto client_status = game::game.GetClientStatus(Name().data());
								if (!client_status.HasHostages()) {
									behavior::rescue::ct_try->Evaluate(this);
								} else {
									behavior::rescue::ct_leave->Evaluate(this);
								}
							}
						},
						{
							game::MapFlags::Assassination,
							[&] {
								auto client_status = game::game.GetClientStatus(Name().data());
								if (client_status.IsVIP()) {
									behavior::assist::ct_vip_escape->Evaluate(this);
								} else {

								}
							}
						},
						{
							game::MapFlags::Escape,
							[&] {
							
							}
						}
					};

					auto current_mode = game::game.GetMapFlag();
					for (auto supported_mode : modes) {
						modes[current_mode & supported_mode.first]();
					}
					break;
				}
			}
		}

		void Bot::Combat() POKEBOT_NOEXCEPT {
			auto client_status = game::game.GetClientStatus(Name().data());
			assert(!target_enemies.empty() && "The bot has no enemies despite being in combat mode.");
			/*
				Choose to fight or to flee.
				The bot has 'fighting spirit' that affects willingness to fight.
			*/

			constexpr int Max_Health = 100;
			constexpr int Max_Armor = 100;
			constexpr int Base_Fearless = Max_Health / 2 + Max_Armor / 5;

			auto enemy_status = game::game.GetClientStatus(target_enemies.front());
			const bool Enemy_Has_Primary = enemy_status.HasPrimaryWeapon();
			const bool Enemy_Has_Secondary = enemy_status.HasSecondaryWeapon();

			int fighting_spirit = Max_Health + Max_Armor + (HasPrimaryWeapon() ? 50 : 0) + (HasSecondaryWeapon() ? 50 : 0);
			if (!HasPrimaryWeapon() && !HasSecondaryWeapon() || 
				(client_status.IsOutOfClip() && client_status.IsOutOfCurrentWeaponAmmo())) {
				// If I have no guns.
				if (Enemy_Has_Primary || Enemy_Has_Secondary) {
					// The enemy has weapon so I should flee.
					fighting_spirit = std::numeric_limits<decltype(fighting_spirit)>::min();
				} else {
					// This is the good chance to beat enemies.
				}
			} else {
				if (IsReloading() || client_status.IsOutOfClip()) {
					// I'm reloading so I have to flee.
					fighting_spirit = std::numeric_limits<decltype(fighting_spirit)>::min();
				} else {
					// If I have guns.
					fighting_spirit -= (Max_Health - Health());
					fighting_spirit -= (Max_Armor - client_status.Armor());
					// fighting_spirit -= (Enemy_Has_Primary ? 25 : 0);
					// fighting_spirit -= (Enemy_Has_Secondary ? 25 : 0);
				}
			}

			if (fighting_spirit <= Base_Fearless) {
				behavior::fight::retreat->Evaluate(this);
			}
			behavior::fight::beat_enemies->Evaluate(this);
		}

		void Bot::Follow() POKEBOT_NOEXCEPT {
			const auto Leader_Origin = Manager::Instance().GetLeaderOrigin(JoinedTeam(), JoinedPlatoon());
			const auto Leader_Area = node::czworld.GetNearest(*Leader_Origin);
			if (Leader_Area == nullptr)
				return;

			if (goal_node == node::Invalid_NodeID) {
				goal_node = Leader_Area->m_id;
			}

			if (!routes.Empty() && !routes.IsEnd()) {
				const auto Current_Area = node::czworld.GetNearest(Origin());
				if (Current_Area == nullptr || Current_Area == Leader_Area) {
					return;
				}

				// - Check -
				const auto Current_Node_ID = Current_Area->m_id;
				if (goal_node == Current_Node_ID) {
					// The bot is already reached at the destination.
					goal_queue.Clear();
					routes.Clear();
					goal_node = node::Invalid_NodeID;
					return;
				}

				if (next_dest_node == node::Invalid_NodeID) {
					next_dest_node = routes.Current();
				} else if (node::czworld.IsOnNode(Origin(), next_dest_node)) {
					if (!routes.IsEnd()) {
						if (routes.Next()) {
							next_dest_node = routes.Current();
						}
					}
				}
			} else {
				node::czworld.FindPath(&routes, Origin(), node::czworld.GetOrigin(goal_node), JoinedTeam());
				if (routes.Empty()) {
					goal_node = node::Invalid_NodeID;
				}
			}
		}

		template<typename Array>
		std::map<float, int> SortedDistances(const Vector& Base, const Array& list) {
			std::map<float, int> result{};
			for (int i = 0; i < list.size(); i++) {
				result[common::Distance(Base, game::game.GetClientStatus(list[i]).origin())] = i;
			}
			return result;
		}

		void Bot::CheckAround() {
			if (!look_direction.view.has_value()) {
#if !USE_NAVMESH
				look_direction.view = node::world.GetOrigin(next_dest_node);
#else
				look_direction.view = node::czworld.GetOrigin(next_dest_node);
				look_direction.view->z = Origin().z;
#endif
			}

			auto next_origin = node::czworld.GetOrigin(next_dest_node);
			if (!look_direction.movement.has_value()) {
#if !USE_NAVMESH
				look_direction.movement = node::world.GetOrigin(next_dest_node);
#else
				look_direction.movement = next_origin;
				look_direction.movement->z = Origin().z;
#endif
			}

			TurnViewAngle();
			TurnMovementAngle();
#if 1
			if (game::poke_fight) {
				auto status = game::game.GetClientStatus(Name().data());
				std::string enemies_in_view[32]{};
				int i{};
				for (const auto& target : status.GetEnemyNamesWithinView()) {
					enemies_in_view[i++] = target;
				}

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

		void Bot::PressKey(ActionKey pressable_key) {
			auto client_status = game::game.GetClientStatus(Name().data());
			if (bool(pressable_key & ActionKey::Run)) {
				move_speed = game::Default_Max_Move_Speed;
			}

			if (bool(pressable_key & ActionKey::Move_Right)) {
				strafe_speed = game::Default_Max_Move_Speed;
			}

			if (bool(pressable_key & ActionKey::Move_Left)) {
				strafe_speed = -game::Default_Max_Move_Speed;
			}
			
			if (bool(pressable_key & ActionKey::Back)) {

			}

			if (bool(pressable_key & ActionKey::Attack)) {

			}
			if (bool(pressable_key & ActionKey::Attack2)) {

			}
			if (bool(pressable_key & ActionKey::Use)) {

			}
			if (bool(pressable_key & ActionKey::Jump)) {
				if (!client_status.IsOnFloor()) {
					return;
				}
			}
			if (bool(pressable_key & ActionKey::Duck)) {

			}

			if (bool(pressable_key & ActionKey::Shift)) {
				pressable_key &= ~ActionKey::Shift;
				move_speed = game::Default_Max_Move_Speed / 2.0f;
			}
			committer.PressKey(static_cast<int>(pressable_key));
		}

		bool Bot::IsPressingKey(const ActionKey Key) const POKEBOT_NOEXCEPT {
			auto client_status = game::game.GetClientStatus(Name().data());
			return (client_status.Button() & static_cast<int>(Key));
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
					assert(JoinedTeam() != common::Team::Spector && JoinedTeam() != common::Team::Random);
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
			game::game.IssueCommand(Name().data(), std::format("menuselect {}", value));
		}

		void Bot::SelectWeapon(const game::Weapon Target_Weapon) {
			if (HasWeapon(Target_Weapon)) {
				current_weapon = Target_Weapon;
				game::game.IssueCommand(Name().data(), std::format("{}", std::get<game::WeaponName>(game::Weapon_CVT[static_cast<int>(Target_Weapon) - 1])));
			}
		}

		void Bot::SelectPrimaryWeapon() {
			SelectWeapon(static_cast<game::Weapon>(std::log2(game::game.GetClientStatus(Name().data()).weapons() & game::Primary_Weapon_Bit)));
		}

		void Bot::SelectSecondaryWeapon() {
			SelectWeapon(static_cast<game::Weapon>(std::log2(game::game.GetClientStatus(Name().data()).weapons() & game::Secondary_Weapon_Bit)));
		}

		void Bot::LookAtClosestEnemy() {
			if (target_enemies.empty()) {
				return;
			}
			const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
			const auto& Nearest_Enemy = game::game.GetClientStatus(target_enemies[Enemy_Distances.begin()->second]);
			look_direction.view = Nearest_Enemy.origin()  - Vector(20.0f, 0, 0) + Manager::Instance().GetCompensation(Name().data());
		}

		bool Bot::HasEnemy() const POKEBOT_NOEXCEPT {
			return !target_enemies.empty();
		}

		bool Bot::IsLookingAtEnemy() const POKEBOT_NOEXCEPT {
			if (target_enemies.empty()) {
				return false;
			}

			const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
			const auto& Nearest_Enemy = game::game.GetClientStatus(target_enemies[Enemy_Distances.begin()->second]);
			return IsLookingAt(Nearest_Enemy.origin(), 1.0f);
		}

		bool Bot::IsEnemyFar() const POKEBOT_NOEXCEPT {
			if (target_enemies.empty()) {
				return false;
			}
			const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
			const auto& Nearest_Enemy = game::game.GetClientStatus(target_enemies[Enemy_Distances.begin()->second]);
			return common::Distance(Origin(), Nearest_Enemy.origin()) > 1000.0f;
		}

		bool Bot::IsLookingAt(const Vector& Dest, const float Range) const POKEBOT_NOEXCEPT {
			float vecout[3]{};
			Vector angle = Dest - Origin();
			VEC_TO_ANGLES(angle, vecout);
			if (vecout[0] > 180.0f)
				vecout[0] -= 360.0f;
			vecout[0] = -vecout[0];
			auto result = common::Distance2D(Vector(vecout), game::game.GetClientStatus(Name().data()).v_angle());
			return (result <= Range);
		}

		std::vector<game::client::Name> Bot::GetEnemyNamesWithinView() const POKEBOT_NOEXCEPT {
			const game::ClientStatus status{ Name().data() };
			return status.GetEnemyNamesWithinView();
		}

		bool Bot::HasGoalToHead() const POKEBOT_NOEXCEPT {
			return goal_node != node::Invalid_NodeID;
		}

		
		bool Bot::IsInBuyzone() const POKEBOT_NOEXCEPT { return game::game.GetClientStatus(Name().data()).IsInBuyzone(); }

		const Name& Bot::Name() const POKEBOT_NOEXCEPT { return name; }

		Vector Bot::Origin() const POKEBOT_NOEXCEPT {
			return game::game.GetClientStatus(Name().data()).origin();
		}

		float Bot::Health() const POKEBOT_NOEXCEPT {
			return game::game.GetClientStatus(Name().data()).Health();
		}

		bool Bot::HasPrimaryWeapon() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).HasPrimaryWeapon()); }
		bool Bot::HasSecondaryWeapon() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).HasSecondaryWeapon()); }
		bool Bot::HasWeapon(const game::Weapon Weapon_ID) const POKEBOT_NOEXCEPT { return game::game.GetClientStatus(Name().data()).HasWeapon(Weapon_ID); }
		bool Bot::IsDucking() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).IsDucking()); }
		bool Bot::IsDriving() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).IsOnTrain()); }
		bool Bot::IsInWater() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).IsInWater()); }
		bool Bot::IsSwimming() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).IsInWater()); }
		bool Bot::IsOnFloor() const POKEBOT_NOEXCEPT { return (game::game.GetClientStatus(Name().data()).IsOnFloor()); }
		bool Bot::IsClimbingLadder() const POKEBOT_NOEXCEPT { return game::game.GetClientStatus(Name().data()).IsClimblingLadder(); }
		bool Bot::IsReloading() const POKEBOT_NOEXCEPT { return game::game.GetClientStatus(Name().data()).IsPlayerModelReloading(); }
		bool Bot::IsPlantingBomb() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsChangingWeapon() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsFalling() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::Jumped() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsJumping() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsLeadingHostages() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsLookingThroughScope() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsLookingThroughCamera() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsChangingSilencer() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsEnabledFlashlight() const POKEBOT_NOEXCEPT { return false; }
		bool Bot::IsEnabledNightvision() const POKEBOT_NOEXCEPT { return false; }

		uint8_t Bot::ComputeMsec() POKEBOT_NOEXCEPT {
			return static_cast<std::uint8_t>((gpGlobals->time - last_command_time) * 1000.0f);
		}

		void Bot::OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) POKEBOT_NOEXCEPT {
			static bool is_sent{};
			static const std::unordered_map<std::string, std::function<void()>> Radios{
				{ 
					"#Cover_me",
					[] {
						
					} 
				},
				{ 
					"#You_take_the_point",
					[] {
						
					} 
				},
				{ 
					"#Hold_this_position",
					[] {
				
					} 
				},
				{ 
					"#Regroup_team", 
					[&] {
#if !USE_NAVMESH
						goal_queue.AddGoalQueue(node::world.GetNearest(game::game.clients.Get(Sender_Name)->origin));
#else
						goal_queue.AddGoalQueue(node::czworld.GetNearest(Origin())->m_id);
#endif
					} 
				},
				{ 
					"#Follow_me", 
					[] {
				
					} 
				},
				{
					"#Taking_fire", 
					[] {
				
					} 
				},
				{
					"#Go_go_go", 
					[this] {
						game::game.IssueCommand(Name().data(), std::format("radio3"));
						game::game.IssueCommand(Name().data(), std::format("menuselect 1"));
					} 
				},
				{ 
					"#Team_fall_back", 
					[] {
				
					} 
				},
				{ 
					"#Stick_together_team", 
					[] {
				
					} 
				},
				{ 
					"#Get_in_position_and_wait", 
					[] {
				
					} 
				},
				{ 
					"#Storm_the_front", [] {
				
					} 
				},
				{ 
					"#Report_in_team", [] {

					} 
				},
				{ "#Affirmative", [] {}},
				{ "#Roger_that", [] {} },
				{
					"#Enemy_spotted", [] {
				
					} 
				},
				{ 
					"#Need_backup", [] {
				
					} 
				},
				{ 
					"#Sector_clear", [] {
				
					} 
				},
				{ 
					"#In_position", [] {
				
					} 
				},
				{ 
					"#Reporting_in", [] {
				
					} 
				},
				{ 
					"#Get_out_of_there", [] {
				
					} 
				},
				{ 
					"#Negative", [] {
					
					} 
				},
				{ 
					"#Enemy_down", [] {
					
					} 
				},
				{ 
					"#Fire_in_the_hole", [] {} 
				}
			};
			Radios.at(Radio_Sentence)();
		}

		
		void Bot::OnBombPlanted() POKEBOT_NOEXCEPT {
			switch (JoinedTeam()) {
				case common::Team::CT:
					goal_queue.Clear();
					break;
			}
		}

		Bot::Bot(const std::string& Bot_Name, const common::Team Join_Team, const common::Model Select_Model) POKEBOT_NOEXCEPT {
			team = Join_Team;
			model = Select_Model;

			name = Bot_Name;

			OnNewRound();
		}

		PlatoonID Bot::JoinedPlatoon() const POKEBOT_NOEXCEPT {
			return platoon;
		}
		
		common::Team Bot::JoinedTeam() const POKEBOT_NOEXCEPT {
			return team;
		}
		
		void Bot::JoinPlatoon(const PlatoonID Target_Platoon) noexcept {
			assert(Target_Platoon.has_value());
			platoon = *Target_Platoon;
		}

		void Bot::ReceiveCommand(const TroopsStrategy& Received_Strategy) {
			switch (Received_Strategy.strategy) {
				case TroopsStrategy::Strategy::Follow:
					assert(!Received_Strategy.leader_name.empty());
					break;
				default:
					goal_queue.AddGoalQueue(Received_Strategy.objective_goal_node, 1);
					break;
			}
			// SERVER_PRINT(std::format("[POKEBOT]New Goal ID:{}\n", goal_node).c_str());
		}
	

		Manager::Manager() :
			troops{
				Troops(
					[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::T; },
					[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::T; },
					common::Team::T),
				Troops(
					[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; },
					[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; },
					common::Team::CT)
			}
		{

		}

		void Manager::OnNewRoundPreparation() POKEBOT_NOEXCEPT {
			for (auto& bot : bots) {
				bot.second.OnNewRound();
			}
			c4_origin = std::nullopt;
			initialization_stage = InitializationStage::Player_Action_Ready;
			round_started_timer.SetTime(1.0f);
		}
		
		void Manager::OnNewRoundReady() POKEBOT_NOEXCEPT {
			if (!round_started_timer.IsRunning() && initialization_stage != InitializationStage::Player_Action_Ready) {
				return;
			}

			for (auto& troop : troops) {
				troop.DecideStrategy(&bots);
				troop.Command(&bots);

				for (auto& platoon : troop) {
					platoon.DecideStrategy(&bots);
					platoon.Command(&bots);
				}
			}
			initialization_stage = InitializationStage::Completed;
		}

		bool Manager::IsExist(const std::string& Bot_Name) const POKEBOT_NOEXCEPT {
			auto it = bots.find(Bot_Name);
			return (it != bots.end());
		}

		void Manager::Assign(const std::string_view Bot_Name, Message message) POKEBOT_NOEXCEPT {
			if (auto target = Get(Bot_Name.data()); target != nullptr) {
				target->start_action = message;
			}
		}

		void Manager::OnDied(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
			auto bot = Get(Bot_Name);
			if (bot != nullptr) {
				bot->current_weapon = game::Weapon::None;
				bot->goal_queue.Clear();
			}
		}

		void Manager::OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) POKEBOT_NOEXCEPT {
			if (decltype(auto) target = Get(Bot_Name.data()); target->Health() <= 0) {
				OnDied(Bot_Name.data());
			} else {
				// TODO: Send the event message for a bot.
			}
		}

		void Manager::OnJoinedTeam(const std::string&) POKEBOT_NOEXCEPT {

		}

		void Manager::OnChatRecieved(const std::string&) POKEBOT_NOEXCEPT {

		}

		void Manager::OnTeamChatRecieved(const std::string&) POKEBOT_NOEXCEPT{

		}

		void Manager::OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) POKEBOT_NOEXCEPT {
			// TODO: Get Sender's team
			radio_message.team = game::game.GetClientStatus(Sender_Name).GetTeam();
			radio_message.sender = Sender_Name;
			radio_message.message = Radio_Sentence;
		}

		void Manager::Insert(std::string bot_name, const common::Team team, const common::Model model, const bot::Difficult Assigned_Diffcult) POKEBOT_NOEXCEPT {
			if (auto spawn_result = game::game.Spawn(bot_name); std::get<bool>(spawn_result)) {
				bot_name = std::get<std::string>(spawn_result);
				auto insert_result = bots.insert({ bot_name, Bot(bot_name, team, model) });
				assert(insert_result.second);

				auto result = balancer.insert({ bot_name, BotBalancer{.gap = {} } });
				assert(result.second);

				switch (Assigned_Diffcult) {
					case bot::Difficult::Easy:
						result.first->second.gap.z = -10.0f;
						break;
					case bot::Difficult::Normal:
						result.first->second.gap.z = -5.0f;
						break;
					case bot::Difficult::Hard:
						break;
					default:
						assert(false);
				}
			}
		}

		void Manager::Update() POKEBOT_NOEXCEPT {
			if (bots.empty())
				return;

			OnNewRoundReady();

			if (!c4_origin.has_value()) {
				edict_t* c4{};
				while ((c4 = common::FindEntityByClassname(c4, "grenade")) != nullptr) {
					if (std::string(STRING(c4->v.model)) == "models/w_c4.mdl") {
						c4_origin = c4->v.origin;
						break;
					}
				}
				if (c4_origin.has_value()) {
					OnBombPlanted();
				}
			}

			for (auto& bot : bots) {
				bot.second.Run();
				if (!radio_message.sender.empty() && radio_message.team == bot.second.JoinedTeam()) {
					bot.second.OnRadioRecieved(radio_message.sender, radio_message.message);
					radio_message.sender.clear();
				}
			}
		}

		Bot* const Manager::Get(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
			auto bot_iterator = bots.find(Bot_Name);
			return (bot_iterator != bots.end() ? &bot_iterator->second : nullptr);
		}

		void Manager::Kick(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
			(*g_engfuncs.pfnServerCommand)(std::format("kick \"{}\"", Bot_Name).c_str());
		}

		void Manager::Remove(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
			if (auto bot_iterator = bots.find(Bot_Name); bot_iterator != bots.end()) {
				bots.erase(Bot_Name);
				balancer.erase(Bot_Name);
			}
		}
		
		void Manager::OnBombPlanted() POKEBOT_NOEXCEPT {
			for (auto& bot : bots) {
				bot.second.OnBombPlanted();
			}
		}

		void Manager::OnBombPickedUp(const std::string& Client_Name) POKEBOT_NOEXCEPT {
			bomber_name = Client_Name;
		}
		
		void Manager::OnBombDropped(const std::string& Client_Name) POKEBOT_NOEXCEPT {
			bomber_name = "";
		}

		void Manager::OnBotJoinedCompletely(Bot* const completed_guy) POKEBOT_NOEXCEPT {
			assert(completed_guy->JoinedTeam() == common::Team::T || completed_guy->JoinedTeam() == common::Team::CT);
			const int Team_Index = static_cast<int>(completed_guy->JoinedTeam()) - 1;
			
			if (auto& troop = troops[Team_Index]; troop.NeedToDevise()) {
				troop.DecideStrategy(&bots);
				troop.Command(&bots);
			}
		}

		node::NodeID Manager::GetGoalNode(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
			auto& troop = troops[static_cast<int>(Target_Team) - 1];
			if (!Index.has_value()) {
				return troop.GetGoalNode();
			} else {
				return troop.at(*Index).GetGoalNode();
			}
		}

		std::optional<Vector> Manager::GetLeaderOrigin(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
			auto& troop = troops[static_cast<int>(Target_Team) - 1];
			if (!Index.has_value()) {
				return troop.CurrentLeaderOrigin();
			} else {
				return troop.at(*Index).CurrentLeaderOrigin();
			}
		}

		bool Manager::IsFollowerPlatoon(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
			if (!Index.has_value()) {
				return false;
			} else {
				auto& troop = troops[static_cast<int>(Target_Team) - 1];
				return troop.at(*Index).IsStrategyToFollow();
			}
		}
	
		bool Troops::HasGoalBeenDevised(const node::NodeID target_objective_node) const POKEBOT_NOEXCEPT {
			return old_strategy.objective_goal_node == target_objective_node;
			// return common::Distance(node::czworld.GetOrigin(old_strategy.objective_goal_node), node::czworld.GetOrigin(target_objective_node)) <= 500.0f;
		}
		bool Troops::HasGoalBeenDevisedByOtherPlatoon(const node::NodeID target_objective_node) const POKEBOT_NOEXCEPT {
			for (auto& platoon : parent->platoons) {
				if (&platoon == this)
					continue;

				if (platoon.strategy.objective_goal_node == target_objective_node) {
					return true;
				}
			}
			return false;
		}
		
		bool Troops::NeedToDevise() const POKEBOT_NOEXCEPT {
			return strategy.objective_goal_node == node::Invalid_NodeID;
		}

		Vector Troops::CurrentLeaderOrigin() const POKEBOT_NOEXCEPT {
			assert(!strategy.leader_name.empty());
			auto leader_status = game::game.GetClientStatus(strategy.leader_name);
			return leader_status.origin();
		}

		int Troops::CreatePlatoon(decltype(condition) target_condition, decltype(condition) target_commander_condition) {
			platoons.push_back({ target_condition, target_commander_condition, Team()});
			platoons.back().parent = this;
			return platoons.size() - 1;
		}

		bool Troops::DeletePlatoon(const int Index) {
			return !platoons.empty() && platoons.erase(platoons.begin() + Index) != platoons.end();
		}

		void Troops::DecideStrategy(std::unordered_map<std::string, Bot>* const bots) {
			if (bots->empty())
				return;

#if 1
			TroopsStrategy new_strategy{};
			auto selectGoal = [&](node::GoalKind kind)->node::NodeID {
#if !USE_NAVMESH
				auto goal = node::world.GetGoal(kind);
#else
				auto goal = node::czworld.GetGoal(kind);
#endif
				for (auto it = goal.first; it != goal.second; it++) {
					if (IsRoot()) {
						if (HasGoalBeenDevised(it->second)) {
							continue;
						}
					} else {
						if (HasGoalBeenDevisedByOtherPlatoon(it->second)) {
							continue;
						}
					}
					return it->second;
				}
			};

			auto candidates = (*bots | std::views::filter(commander_condition));
			if (candidates.empty())
				return;

			Bot* leader = &candidates.front().second;
			node::GoalKind kind{};
			if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
				kind = node::GoalKind::Bombspot;

				switch (leader->JoinedTeam()) {
					case common::Team::T:
					{
						new_strategy.strategy = TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite_Concentrative;
						switch (new_strategy.strategy) {
							case TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite_Concentrative:
							{
								if (IsRoot()) {
									// If the troop is the root, create new platoons.
									while (DeletePlatoon(0));	// Delete all platoon.
									
									int platoon = CreatePlatoon(
										[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedPlatoon() == 0; },
										[](const std::pair<std::string, Bot>& target) -> bool { return target.second.HasWeapon(game::Weapon::C4); }
									);

                                    auto followers = (*bots | std::views::filter([](const std::pair<std::string, Bot>& target) -> bool { 
										return target.second.JoinedTeam() == common::Team::T && !target.second.HasWeapon(game::Weapon::C4); }) | std::views::take(5)
									);
									for (auto& follower : followers) {
										follower.second.JoinPlatoon(platoon);
									}
									new_strategy.objective_goal_node = selectGoal(kind);
								} else {
									// If the troop is a platoon
									new_strategy.strategy = TroopsStrategy::Strategy::Follow;
									new_strategy.leader_name = Manager::Instance().Bomber_Name;
									assert(!new_strategy.leader_name.empty());
								}
								break;
							}
							default:
								assert(false);
								break;
						}
						break;
					}
					case common::Team::CT:
					{
						new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
						switch (new_strategy.strategy) {
							case TroopsStrategy::Strategy::Defend_Bombsite_Divided:
							{
								if (IsRoot()) {
									// If the troop is the root, create new platoons.
									while (DeletePlatoon(0));	// Delete all platoon.

									const size_t Number_Of_Goals = node::czworld.GetNumberOfGoals(node::GoalKind::Bombspot);
									assert(Number_Of_Goals > 0);
									for (int i = 0; i < Number_Of_Goals; i++) {
										CreatePlatoon(
											[i](const std::pair<std::string, Bot>& target) -> bool {
											return i == target.second.JoinedPlatoon();
										},
											[i](const std::pair<std::string, Bot>& target) -> bool {
											return i == target.second.JoinedPlatoon();
										}
										);
									}

									auto cts = (*bots | std::views::filter([](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; }));
									const size_t Number_Of_Cts = std::distance(cts.begin(), cts.end());
									if (Number_Of_Cts > 1) {
										const size_t Number_Of_Member_In_Squad = static_cast<size_t>(std::ceil(static_cast<common::Dec>(Number_Of_Cts) / static_cast<common::Dec>(Number_Of_Goals)));
										auto member = cts.begin();
										for (int squad = 0; squad < Number_Of_Goals; squad++) {
											for (int j = 0; j < Number_Of_Member_In_Squad && member != cts.end(); j++, member++) {
												member->second.JoinPlatoon(squad);
											}
										}
									} else {
										new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
										new_strategy.objective_goal_node = selectGoal(kind);
									}
								} else {
									// If the troop is a platoon
									new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Divided;
									new_strategy.objective_goal_node = selectGoal(kind);
								}
								break;
							}
							default:
								assert(false);
								break;
						}
						break;
					}
				}
			} else if (game::game.IsCurrentMode(game::MapFlags::HostageRescue)) {
				kind = node::GoalKind::Rescue_Zone;

				switch (leader->JoinedTeam()) {
					case common::Team::T:
					{
						strategy.strategy = TroopsStrategy::Strategy::Prevent_Hostages;
						new_strategy.objective_goal_node = selectGoal(kind);
						break;
					}
					case common::Team::CT:
					{
						strategy.strategy = TroopsStrategy::Strategy::Rush_And_Rescue;
						new_strategy.objective_goal_node = selectGoal(kind);
						break;
					}
				}
			} else if (game::game.IsCurrentMode(game::MapFlags::Assassination)) {
				kind = node::GoalKind::Vip_Safety;
				new_strategy.objective_goal_node = selectGoal(kind);
			} else if (game::game.IsCurrentMode(game::MapFlags::Escape)) {
				kind = node::GoalKind::Escape_Zone;
				new_strategy.objective_goal_node = selectGoal(kind);
			}
			SetNewStrategy(new_strategy);
#else
			auto candidates = (*bots | std::views::filter(commander_condition));
			if (candidates.empty())
				return;

			switch (Bot* leader = &candidates.front().second; leader->JoinedTeam()) {
				case common::Team::T:
					DecideStrategyForT(bots);
					break;
				case common::Team::CT:
					break;
			}
#endif
		}

		void Troops::Command(std::unordered_map<std::string, Bot>* bots) {
			for (auto& individual : (*bots | std::views::filter(condition))) {
				individual.second.ReceiveCommand(strategy);
			}
		}

		void Troops::SetNewStrategy(const TroopsStrategy& New_Team_Strategy) {
			old_strategy = strategy;
			strategy = New_Team_Strategy;

			for (auto& platoon : platoons) {
				platoon.SetNewStrategy(strategy);
			}
		}
	}
}