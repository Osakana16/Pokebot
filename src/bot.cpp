#include "behavior.hpp"

#include <thread>

namespace pokebot {
	namespace bot {
		void Bot::Run() POKEBOT_DEBUG_NOEXCEPT {
			const static std::unordered_map<Message, std::function<void(Bot&)>> Update_Funcs{
				{ Message::Team_Select, &Bot::SelectionUpdate },
				{ Message::Model_Select, &Bot::SelectionUpdate },
				{ Message::Buy, &Bot::BuyUpdate },
				{ Message::Normal, &Bot::NormalUpdate },
				{ Message::Selection_Completed, &Bot::OnSelectionCompleted }
			};

			if (*client == nullptr)
				return; 

			auto update = Update_Funcs.at(start_action);
			update(*this);
			frame_interval = gpGlobals->time - last_command_time;

			const std::uint8_t Msec_Value = ComputeMsec();
			last_command_time = gpGlobals->time;
			g_engfuncs.pfnRunPlayerMove(*client,
					movement_angle,
					move_speed,
					strafe_speed,
					0.0f,
					client->Button(),
					client->Impulse(),
					Msec_Value);

			client->Edict()->v.flags |= pokebot::common::Third_Party_Bot_Flag;
			move_speed = 0;
		}

#define ENABLE_NEW_TURN_ANGLE 1

		void Bot::TurnViewAngle() {
			auto destination = look_direction.view->ToAngleVector(Origin());
			if (destination.x > 180.0f) {
				destination.x -= 360.0f;
			}
#if ENABLE_NEW_TURN_ANGLE
			
			client->v_angle().x = destination.x;

			if (std::abs(destination.y - client->v_angle().y) <= 1.0f) {
				client->v_angle().y = destination.y;
			} else {
				auto AngleClamp = [](const float angle, const float destination) {
					return (angle > destination ? std::clamp(angle, destination, angle) : std::clamp(angle, angle, destination));
				};

				auto CalculateNextAngle = [](const float dest, const float angle) noexcept {
					return std::clamp(dest - angle, -180.0f, 180.0f);
				};

				constexpr float Base_Frame = 30.0f;
				constexpr float Sensitivity = 1.0f;
				const common::AngleVector Next_Angle = {
					CalculateNextAngle(destination.x, client->v_angle().x) / (Base_Frame - Sensitivity),
					CalculateNextAngle(destination.y, client->v_angle().y) / (Base_Frame - Sensitivity),
					0.0
				};

				client->v_angle().y += Next_Angle.y;
				client->v_angle().y = AngleClamp(client->v_angle().y, destination.y);
			}

#else
			client->v_angle() = destination;
#endif
			client->v_angle().z = 0.0f;
			client->angles().x = client->v_angle().x / 3;
			client->angles().y = client->v_angle().y;
			client->v_angle().x = -client->v_angle().x;
			client->angles().z = 0;
			client->ideal_yaw() = client->v_angle().y;
			if (client->ideal_yaw() > 180.0f) {
				client->ideal_yaw() -= 360.0f;
			} else if (client->ideal_yaw() < -180.0f) {
				client->ideal_yaw() += 360.0f;
			}

			client->idealpitch() = client->v_angle().x;
			if (client->idealpitch() > 180.0f) {
				client->idealpitch() -= 360.0f;
			} else if (client->idealpitch() < -180.0f) {
				client->idealpitch() += 360.0f;
			}

		}

		void Bot::TurnMovementAngle() {
			movement_angle = look_direction.movement->ToAngleVector(Origin());
		}

		void Bot::OnNewRound() POKEBOT_DEBUG_NOEXCEPT {
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

		void Bot::NormalUpdate() noexcept {
			assert(JoinedTeam() != common::Team::Spector && JoinedTeam() != common::Team::Random);
			if (client->IsDead()) {
				return;
			}
			
			const int current_weapon_integer = static_cast<int>(current_weapon);
			assert(current_weapon_integer >= static_cast<int>(game::Weapon::None) &&  current_weapon_integer <= static_cast<int>(game::Weapon::P90));  
				
			if (manager.C4Origin().has_value()) {
				need_to_update = true;
			}

			if (!update_timer.IsRunning()) {
				need_to_update = true;
				update_timer.SetTime(30.0);
			}

			if (need_to_update) {
				if (!allow_listen_radio_timer.IsRunning()) {
					can_listen_radio = true;
					can_follow_radio = true;
					allow_listen_radio_timer.SetTime(30.0);
				}
				need_to_update = false;
			}

			if (!freeze_time.IsRunning() && !spawn_cooldown_time.IsRunning()) {
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
				}
				CheckAround();
			} else {
				SelectWeapon(game::Weapon::Knife);
			}

			for (auto& vector : { Vector(0.0f, 0.0f, 0.0f), Vector(50.0f, 0.0f, 0.0f), Vector(-50.0f, 0.0f, 0.0f), Vector(0.0f, 50.0f, 0.0f), Vector(0.0f, -50.0f, 0.0f) }) {
				if (auto area = node::czworld.GetNearest(Origin() + vector); area != nullptr && node::czworld.HasFlag(area->m_id, node::NavmeshFlag::Jump)) {
					PressKey(ActionKey::Jump);
					break;
				}
			}
			if (next_dest_node != node::Invalid_NodeID) {
				PressKey(ActionKey::Run);
			}
		}

		void Bot::OnSelectionCompleted() noexcept {
			manager.OnBotJoinedCompletely(this);
			start_action = Message::Buy;
		}

		void Bot::AccomplishMission() noexcept {
			static const auto Do_Nothing_If_Mode_Is_Not_Applicable = std::make_pair<game::MapFlags, std::function<void()>>(static_cast<game::MapFlags>(0), [this] { /* Do nothing. */ });
			switch (JoinedTeam()) {
				case common::Team::T:
				{
					std::unordered_map<game::MapFlags, std::function<void()>> modes{
						Do_Nothing_If_Mode_Is_Not_Applicable,
						{
							game::MapFlags::Demolition,
							[this] {
								if (manager.C4Origin().has_value()) {
									behavior::demolition::t_planted_wary->Evalute(this);
								} else {
									if (HasWeapon(game::Weapon::C4)) {
										behavior::demolition::t_plant->Evalute(this);
									} else {
									}
								}
							}
						},
						{
							game::MapFlags::HostageRescue,
							[this] {
							
							}
						},
						{
							game::MapFlags::Assassination,
							[this] {
							
							}
						},
						{
							game::MapFlags::Escape,
							[this] {
								behavior::escape::t_take_point->Evalute(this);
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
							[this] {
								if (manager.C4Origin().has_value()) {
									if (common::Distance(Origin(), *manager.C4Origin()) <= 50.0f) {
										behavior::demolition::ct_defusing->Evalute(this);
									} else {
										behavior::demolition::ct_planted->Evalute(this);
									}
								} else {
									behavior::demolition::ct_defend->Evalute(this);
								}
							}
						},
						{
							game::MapFlags::HostageRescue,
							[this] {
								if (!client->HasHostages()) {
									behavior::rescue::ct_try->Evalute(this);
								} else {
									behavior::rescue::ct_leave->Evalute(this);
								}
							}
						},
						{
							game::MapFlags::Assassination,
							[this] {
								if (client->IsVIP()) {
									behavior::assist::ct_vip_escape->Evalute(this);
								} else {

								}
							}
						},
						{
							game::MapFlags::Escape,
							[this] {
							
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

		void Bot::Combat() noexcept {
			behavior::fight::while_spotting_enemy->Evalute(this);
		}

		template<typename Array>
		std::map<float, int> SortedDistances(const Vector& Base, const Array& list) {
			std::map<float, int> result{};
			for (int i = 0; i < list.size(); i++) {
				result[common::Distance(Base, (*(list.cbegin() + i))->v.origin)] = i;
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
#endif
			}

			TurnViewAngle();
			TurnMovementAngle();
			look_direction.Clear();
			auto status = game::game.clients.GetClientStatus(client->Name());
			for (auto& entity : entities) {
				entity.clear();
			}
			for (auto& target : status.GetEntitiesInView()) {
				switch (common::GetTeamFromModel(target)) {
					case common::Team::T:
					case common::Team::CT:
						entities[(common::GetTeamFromModel(target) != JoinedTeam())].push_back(target);
						break;
					default:
						break;
				}
			}

			if (!game::poke_fight) {
				entities[ENEMY].clear();
			}
			
			if (!entities[MATE].empty()) {
				const auto Mate_Distances = std::move(SortedDistances(Origin(), entities[MATE]));
			}

			if (!entities[ENEMY].empty()) {
				// danger_time.SetTime(5.0);
				state = State::Crisis;
			} else {
				state = State::Accomplishment;
			}
		}

		void Bot::PressKey(ActionKey pressable_key) {
			if (bool(pressable_key & ActionKey::Run)) {
				move_speed = 255.0;
			}

			if (bool(pressable_key & ActionKey::Move_Right)) {
				strafe_speed = 255.0;
			}

			if (bool(pressable_key & ActionKey::Move_Left)) {
				strafe_speed = -255.0;
			}

			if (bool(pressable_key & ActionKey::Attack)) {

			}
			if (bool(pressable_key & ActionKey::Attack2)) {

			}
			if (bool(pressable_key & ActionKey::Use)) {

			}
			if (bool(pressable_key & ActionKey::Jump)) {
				if (!client->IsOnFloor()) {
					return;
				}
			}
			if (bool(pressable_key & ActionKey::Duck)) {

			}
			if (bool(pressable_key & ActionKey::Shift)) {
				
			}
			client->PressKey(static_cast<int>(pressable_key));
		}

		bool Bot::IsPressingKey(const ActionKey Key) const noexcept {
			return (client->Button() & static_cast<int>(Key));
		}

		void Bot::SelectionUpdate() noexcept {
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
			game::game.IssueCommand(*client, std::format("menuselect {}", value));
		}

		void Bot::SelectWeapon(const game::Weapon Target_Weapon) {
			if (HasWeapon(Target_Weapon)) {
				current_weapon = Target_Weapon;
				game::game.IssueCommand(*client, std::format("{}", std::get<game::WeaponName>(game::Weapon_CVT[static_cast<int>(Target_Weapon) - 1])));
			}
		}

		void Bot::SelectPrimaryWeapon() {
			SelectWeapon(static_cast<game::Weapon>(std::log2(client->Edict()->v.weapons & game::Primary_Weapon_Bit)));
		}

		void Bot::SelectSecondaryWeapon() {
			SelectWeapon(static_cast<game::Weapon>(std::log2(client->Edict()->v.weapons & game::Secondary_Weapon_Bit)));
		}

		void Bot::LookAtClosestEnemy() {
			if (entities[ENEMY].empty()) {
				return;
			}
			const auto Enemy_Distances = std::move(SortedDistances(Origin(), entities[ENEMY]));
			const auto& Nearest_Enemy = entities[ENEMY][Enemy_Distances.begin()->second];
			look_direction.view = Nearest_Enemy->v.origin - Vector(20.0f, 0, 0) + manager.GetCompensation(Name().data());
		}

		bool Bot::IsLookingAtEnemy() const noexcept {
			if (entities[ENEMY].empty()) {
				return false;
			}

			const auto Enemy_Distances = std::move(SortedDistances(Origin(), entities[ENEMY]));
			const auto& Nearest_Enemy = entities[ENEMY][Enemy_Distances.begin()->second];
			return IsLookingAt(Nearest_Enemy->v.origin, 1.0f);
		}

		bool Bot::IsEnemyFar() const noexcept {
			if (entities[ENEMY].empty()) {
				return false;
			}
			const auto Enemy_Distances = std::move(SortedDistances(Origin(), entities[ENEMY]));
			const auto& Nearest_Enemy = entities[ENEMY][Enemy_Distances.begin()->second];
			return common::Distance(Origin(), Nearest_Enemy->v.origin) > 1000.0f;
		}

		bool Bot::IsLookingAt(const Vector& Dest, const float Range) const noexcept {
			float vecout[3]{};
			Vector angle = Dest - Origin();
			VEC_TO_ANGLES(angle, vecout);
			if (vecout[0] > 180.0f)
				vecout[0] -= 360.0f;
			vecout[0] = -vecout[0];
			auto result = common::Distance2D(Vector(vecout), client->v_angle());
			return (result <= Range);
		}

		std::shared_ptr<game::Client> Bot::GetEnemyWithinView() const noexcept {
			const game::ClientStatus status{client};
			return status.GetEnemyWithinView();
		}

		bool Bot::HasGoalToHead() const noexcept {
			return goal_node != node::Invalid_NodeID;
		}

		Vector Bot::Origin() const noexcept {
			return client->origin();
		}

		float Bot::Health() const noexcept {
			return client->Health();
		}

		uint8_t Bot::ComputeMsec() noexcept {
			return static_cast<std::uint8_t>((gpGlobals->time - last_command_time) * 1000.0f);
		}

		void Bot::OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) noexcept {
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
						goal_queue.AddGoalQueue(node::czworld.GetNearest(game::game.clients.Get(Sender_Name)->origin())->m_id);
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
						game::game.IssueCommand(*client, std::format("radio3"));
						game::game.IssueCommand(*client, std::format("menuselect 1"));
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

		
		void Bot::OnBombPlanted() noexcept {
			switch (JoinedTeam()) {
				case common::Team::CT:
					goal_queue.Clear();
					break;
			}
		}

		
		Manager::Manager() :
			troops{{ 
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::T; },
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::T; },
				common::Team::T 
			},
			{
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; },
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; },
				common::Team::CT
			}}
		{

		}

		void Manager::OnNewRound() {
			for (auto& bot : bots) {
				bot.second.OnNewRound();
			}

			for (auto& troop : troops) {
				troop.DecideStrategy(&bots);
				troop.Command(&bots);

				for (auto& platoon : troop) {
					platoon.DecideStrategy(&bots);
					platoon.Command(&bots);
				}
			}
			c4_origin = std::nullopt;
		}
		
		bool Manager::IsExist(const std::string& Bot_Name) const noexcept {
			auto it = bots.find(Bot_Name);
			return (it != bots.end());
		}

		void Manager::Assign(const std::string_view Bot_Name, Message message) noexcept {
			if (auto target = Get(Bot_Name.data()); target != nullptr) {
				target->start_action = message;
			}
		}

		void Manager::OnDied(const std::string& Bot_Name) noexcept {
			auto bot = Get(Bot_Name);
			if (bot != nullptr) {
				bot->current_weapon = game::Weapon::None;
				bot->goal_queue.Clear();
			}
		}

		void Manager::OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) noexcept {
			if (decltype(auto) target = Get(Bot_Name.data()); target->client->Health() <= 0) {
				OnDied(Bot_Name.data());
			} else {
				// TODO: Send the event message for a bot.
			}
		}

		void Manager::OnJoinedTeam(const std::string&) noexcept {

		}

		void Manager::OnChatRecieved(const std::string&) noexcept {

		}

		void Manager::OnTeamChatRecieved(const std::string&) noexcept{

		}

		void Manager::OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) noexcept {
			// TODO: Get Sender's team
			radio_message.team = game::game.clients.Get(Sender_Name)->GetTeam();
			radio_message.sender = Sender_Name;
			radio_message.message = Radio_Sentence;
		}

		void Manager::Insert(std::string bot_name, const common::Team team, const common::Model model, const bot::Difficult Assigned_Diffcult) POKEBOT_DEBUG_NOEXCEPT {
			auto bot_client = game::game.clients.Create(bot_name);
			bot_name = bot_client->Name();
			bots.insert({ bot_name, Bot(bot_client, team, model) });

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

		void Manager::Update() noexcept {
			if (bots.empty())
				return;

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

		Bot* const Manager::Get(const std::string& Bot_Name) noexcept {
			auto bot_iterator = bots.find(Bot_Name);
			return (bot_iterator != bots.end() ? &bot_iterator->second : nullptr);
		}

		void Manager::Kick(const std::string& Bot_Name) noexcept {
			(*g_engfuncs.pfnServerCommand)(std::format("kick \"{}\"", Bot_Name).c_str());
		}

		void Manager::Remove(const std::string& Bot_Name) noexcept {
			if (auto bot_iterator = bots.find(Bot_Name); bot_iterator != bots.end()) {
				bots.erase(Bot_Name);
				balancer.erase(Bot_Name);
			}
		}
		
		void Manager::OnBombPlanted() noexcept {
			for (auto& bot : bots) {
				bot.second.OnBombPlanted();
			}
		}

		void Manager::OnBotJoinedCompletely(Bot* const completed_guy) noexcept {
			assert(completed_guy->JoinedTeam() == common::Team::T || completed_guy->JoinedTeam() == common::Team::CT);
			const int Team_Index = static_cast<int>(completed_guy->JoinedTeam()) - 1;
			
			if (auto& troop = troops[Team_Index]; troop.NeedToDevise()) {
				troop.DecideStrategy(&bots);
				troop.Command(&bots);
			}
		}

		node::NodeID Manager::GetGoalNode(const common::Team Target_Team, const int Index) const noexcept {
			auto& troop = troops[static_cast<int>(Target_Team) - 1];
			if (Index < 0) {
				return troop.GetGoalNode();
			} else {
				return troop.at(Index).GetGoalNode();
			}
		}

		Bot::Bot(std::shared_ptr<game::Client> assigned_client, const common::Team Join_Team, const common::Model Select_Model) POKEBOT_DEBUG_NOEXCEPT :
			client(assigned_client)
		{
			team = Join_Team;
			model = Select_Model;

			name = client->Name();

			OnNewRound();
		}

		int Bot::JoinedPlatoon() const noexcept {
			return platoon;
		}
		
		common::Team Bot::JoinedTeam() const noexcept {
			return team;
		}

		void Bot::ReceiveCommand(const TroopsStrategy& Received_Strategy) {
			switch (Received_Strategy.strategy) {
				case TroopsStrategy::Strategy::Defend_Bombsite_Divided:
					break;
				default:
					goal_queue.AddGoalQueue(Received_Strategy.objective_goal_node, 1);
					break;
			}
			// SERVER_PRINT(std::format("[POKEBOT]New Goal ID:{}\n", goal_node).c_str());
		}

	
		bool Troops::HasGoalBeenDevised(const node::NodeID target_objective_node) const noexcept {
			return old_strategy.objective_goal_node == target_objective_node;
			// return common::Distance(node::czworld.GetOrigin(old_strategy.objective_goal_node), node::czworld.GetOrigin(target_objective_node)) <= 500.0f;
		}
		bool Troops::HasGoalBeenDevisedByOtherPlatoon(const node::NodeID target_objective_node) const noexcept {
			for (auto& platoon : parent->platoons) {
				if (&platoon == this)
					continue;

				if (platoon.strategy.objective_goal_node == target_objective_node) {
					return true;
				}
			}
			return false;
		}
		
		bool Troops::NeedToDevise() const noexcept {
			return strategy.objective_goal_node == node::Invalid_NodeID;
		}

		int Troops::CreatePlatoon(decltype(condition) target_condition, decltype(condition) target_leader_condition) {
			platoons.push_back({ target_condition, target_leader_condition, Team()});
			platoons.back().parent = this;
			return platoons.size() - 1;
		}

		bool Troops::DeletePlatoon(const int Index) {
			return !platoons.empty() && platoons.erase(platoons.begin() + Index) != platoons.end();
		}

		void Troops::DecideStrategy(std::unordered_map<std::string, Bot>* bots) {
			if (bots->empty())
				return;

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

			auto candidates = (*bots | std::views::filter(leader_condition));
			if (candidates.empty())
				return;

			Bot* leader = &candidates.front().second;
			node::GoalKind kind{};
			if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
				kind = node::GoalKind::Bombspot;

				switch (leader->JoinedTeam()) {
					case common::Team::T:
					{
						new_strategy.strategy = TroopsStrategy::Strategy::Plant_C4_Specific_Bombsite;
						new_strategy.objective_goal_node = selectGoal(kind);
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
									for (int i = 0; i < Number_Of_Goals; i++) {
										CreatePlatoon(
											[i] (const std::pair<std::string, Bot>& target) -> bool {
												return i == target.second.JoinedPlatoon();
											},
											[i] (const std::pair<std::string, Bot>& target) -> bool {
												return i == target.second.JoinedPlatoon();
											}
										);
									}

									auto cts = (*bots | std::views::filter([](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; }));
									const size_t Range = std::distance(cts.begin(), cts.end()) / Number_Of_Goals;
									if (Range > 0) {
										auto size = std::distance(cts.begin(), cts.end());
										for (int i = 0; i < size; i++) {
											auto bot = std::next(cts.begin(), i);
											bot->second.platoon = i / (size % Number_Of_Goals + 1);
										}
									} else {
										new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Concentrative;
										new_strategy.objective_goal_node = selectGoal(kind);
									}
								} else {
									// If the troop is a platoon
									new_strategy.strategy = TroopsStrategy::Strategy::Defend_Bombsite_Concentrative;
									new_strategy.objective_goal_node = selectGoal(kind);
								}
								break;
							}
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