namespace pokebot {
	namespace bot {
		ActionKey operator|(ActionKey ak1, ActionKey ak2) noexcept {
			return static_cast<ActionKey>(static_cast<int>(ak1) | static_cast<int>(ak2));
		}

		ActionKey operator&(ActionKey ak1, ActionKey ak2) noexcept {
			return static_cast<ActionKey>(static_cast<int>(ak1) & static_cast<int>(ak2));
		}

		ActionKey operator^(ActionKey ak1, ActionKey ak2) noexcept {
			return static_cast<ActionKey>(static_cast<int>(ak1) ^ static_cast<int>(ak2));
		}

		void Bot::Run() noexcept {
			const static std::unordered_map<Message, std::function<void(Bot&)>> Update_Funcs{
				{ Message::Team_Select, &Bot::SelectionUpdate },
				{ Message::Model_Select, &Bot::SelectionUpdate },
				{ Message::Buy, &Bot::BuyUpdate },
				{ Message::Normal, &Bot::NormalUpdate }
			};

			client->Edict()->v.flags |= pokebot::common::Third_Party_Bot_Flag;
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

			static_cast<edict_t*>(*client)->v.button = 0;
			move_speed = 0;
		}

#define ENABLE_NEW_TURN_ANGLE 0

		void Bot::TurnViewAngle() {
			auto destination = look_direction.view->ToAngleVector(Origin());
#if ENABLE_NEW_TURN_ANGLE
			auto AngleClamp = [](const float angle, const float destination) {
				return (angle > destination ? std::clamp(angle, destination, angle) : std::clamp(angle, angle, destination));
			};

			auto CalculateNextAngle = [](const float dest, const float angle) {
				auto next = dest - angle;
				if (next < -180.0f)
					next = -180.0f;
				else if (next > 180.0f)
					next = 180.0f;
				return next;
			};

			if (common::Distance2D(destination, client->v_angle) <= 1.0f) {
				return;
			}
			destination.x *= -1;

			constexpr float Base_Frame = 30.0f;
			constexpr float Sensitivity = 1.0f;
			const common::AngleVector Next_Angle = { 
				CalculateNextAngle(destination.x, client->v_angle.x) / (Base_Frame - Sensitivity),
				CalculateNextAngle(destination.y, client->v_angle.y) / (Base_Frame - Sensitivity),
				CalculateNextAngle(destination.z, client->v_angle.z) / (Base_Frame - Sensitivity)
			};

			client->v_angle.x += Next_Angle.x;
			client->v_angle.y += Next_Angle.y;
			client->v_angle.z += Next_Angle.z;
			client->v_angle.x = AngleClamp(client->v_angle.x, destination.x);
			client->v_angle.y = AngleClamp(client->v_angle.y, destination.y);
			client->v_angle.z = AngleClamp(client->v_angle.z, destination.z);
#else
			client->v_angle = destination;
#endif
			client->angles.x = client->v_angle.x;
			client->angles.y = client->v_angle.y;
			client->v_angle.x = -client->v_angle.x;
			client->angles.z = 0;
			client->ideal_yaw = client->v_angle.y;
			client->idealpitch = client->v_angle.x;
		}

		void Bot::TurnMovementAngle() {
			movement_angle = look_direction.movement->ToAngleVector(Origin());
		}

		void Bot::OnNewRound() POKEBOT_DEBUG_NOEXCEPT {
			goal_queue.Clear();
			routes.Clear();

			squad = -1;

			goal_node = node::Invalid_NodeID;
			next_dest_node = node::Invalid_NodeID;
	
			look_direction.Clear();
			ideal_direction.Clear();

			freeze_time.SetTime(g_engfuncs.pfnCVarGetFloat("mp_freezetime") + 1.0f);

			start_action = Message::Buy;
			buy_wait_timer.SetTime(1.0f);

			// mapdata::BSP().LoadWorldMap();
		}

		void Bot::NormalUpdate() noexcept {
			if (client->IsDead()) {
				return;
			}
				
			if (manager.Is_Bomb_Planted) {
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
			
			if (!freeze_time.IsRunning()) {
				// Do not do anything when freezing.
				// Due to game engine specifications or bugs, 
				// if we execute a heavy process immediately after the start of a round, 
				// the game will freeze.
				
				BehaviorUpdate();
			} 
			
			if (next_dest_node != node::Invalid_NodeID) {
				if (move_speed > 0.0) {
					PressKey(ActionKey::Run);
				}
			}
			CheckAround();
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
				look_direction.view = node::world.GetOrigin(next_dest_node);
			}

			if (!look_direction.movement.has_value()) {
				look_direction.movement = node::world.GetOrigin(next_dest_node);
			}

			TurnViewAngle();
			TurnMovementAngle();
			look_direction.Clear();
			auto status = game::game.clients.GetClientStatus(client->Name());
			entities->clear();
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
			
			if (!entities[MATE].empty()) {
				const auto Mate_Distances = std::move(SortedDistances(Origin(), entities[MATE]));
			}

			if (!entities[ENEMY].empty()) {
				danger_time.SetTime(5.0);
			}
#if 0
			for (const auto& other : game::game.clients.GetAll()) {
				if (other.second == client)
					continue;

				if (CanSeeEntity(*other.second)) {
					if (common::GetTeamFromModel(*client) != common::GetTeamFromModel(*other.second)) {
						target.insert(other.second);
					}
				} else {
					target.erase(other.second);
				}
			}

			if (!target.empty()) {
				if ((*target.begin())->IsDead()) {
					target.erase(target.begin());
				} else {
					TurnAngle((*target.begin())->origin);
				}
			}
#endif
		}

		void Bot::PressKey(ActionKey pressable_key) {
			if (bool(pressable_key & ActionKey::Run)) {
				move_speed = 255.0;
			}
			if (bool(pressable_key & ActionKey::Attack)) {

			}
			if (bool(pressable_key & ActionKey::Attack2)) {

			}
			if (bool(pressable_key & ActionKey::Use)) {

			}
			if (bool(pressable_key & ActionKey::Jump)) {

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
					value = static_cast<int>(team);
					break;
				}
				case Message::Model_Select:
				{
					start_action = Message::Buy;
					value = static_cast<int>(model);
					break;
				}
				default:
				{
					return;
				}
			}
			game::game.IssueCommand(*client, std::format("menuselect {}", value));
		}

		void Bot::SelectWeapon(const game::Weapon Target_Weapon) {
			if (HasWeapon(Target_Weapon)) {
				// current_weapon = Target_Weapon;
				game::game.IssueCommand(*client, std::format("{}", game::Weapon_CVT[static_cast<int>(Target_Weapon) - 1]));
			}
		}

		void Bot::LookAtClosestEnemy() {
			const auto Enemy_Distances = std::move(SortedDistances(Origin(), entities[ENEMY]));
			const auto& Nearest_Enemy = entities[ENEMY][Enemy_Distances.begin()->second];
			look_direction.view = Nearest_Enemy->v.origin;
		}

		bool Bot::IsLookingAt(const Vector& Dest) const noexcept {
			return (common::Distance2D(Dest, client->v_angle) <= 1.0f);
		}

		bool Bot::CanSeeEnemy() const noexcept {
			const game::ClientStatus status{client};
			return status.CanSeeEnemy();
		}

		bool Bot::HasGoalToHead() const noexcept {
			return goal_node != node::Invalid_NodeID;
		}

		Vector Bot::Origin() const noexcept {
			return client->origin;
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
					[] {
						
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

		void Manager::OnNewRound() {
			for (auto& squad : squads) {
				squad.clear();
				squad.emplace_back(Squad(STRING(game::game.GetHost()->v.netname), 3, Policy::Player));
			}

			for (auto& bot : bots) {
				bot.second.OnNewRound();
			}
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
				bot->target.clear();
			}
		}

		void Manager::OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) noexcept {
			if (decltype(auto) target = Get(Bot_Name.data()); target->client->Health <= 0) {
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

		void Manager::Insert(const std::string& Bot_Name, const common::Team team, const common::Model model) POKEBOT_DEBUG_NOEXCEPT {
			bots.insert({ Bot_Name, Bot(game::game.clients.Create(Bot_Name), team, model) });
		}

		void Manager::Update() noexcept {
			if (bots.empty())
				return;

			bool is_bomb_found{};
			edict_t* c4{};
			while ((c4 = common::FindEntityByClassname(c4, "grenade")) != nullptr) {
				if (std::string(STRING(c4->v.model)) == "models/w_c4.mdl") {
					is_bomb_found = true;
					c4_origin = c4->v.origin;
					break;
				}
			}
			is_bomb_planted = is_bomb_found;

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
			}
		}

		int Manager::SetupLonelySquad(const std::string& Candidate_Name) {
			int team = (int)bots.at(Candidate_Name).JoinedTeam() - 1;
			assert(team == 0 || team == 1);

			auto& squad = squads[team];
			squad.emplace_back(Squad(Candidate_Name, 1, Policy::Elimination));
			return squad.size() - 1;
		}

		int Manager::SetupVipSquad(const std::string& Candidate_Name) {
			int team = (int)bots.at(Candidate_Name).JoinedTeam() - 1;
			assert(team == 0 || team == 1);

			auto& squad = squads[team];
			squad.emplace_back(Squad(Candidate_Name, 3, Policy::Elimination));
			return squad.size() - 1;
		}

		int Manager::SetupHelperSquad(const std::string& Candidate_Name) {
			int team = (int)bots.at(Candidate_Name).JoinedTeam() - 1;
			assert(team == 0 || team == 1);

			auto& squad = squads[team];
			squad.emplace_back(Squad(Candidate_Name, 3, Policy::Elimination));
			return squad.size() - 1;
		}

		int Manager::SetupDefenseSquad(const std::string& Candidate_Name) {
			int team = (int)bots.at(Candidate_Name).JoinedTeam() - 1;
			assert(team == 0 || team == 1);

			auto& squad = squads[team];
			squad.emplace_back(Squad(Candidate_Name, 3, Policy::Defense));
			return squad.size() - 1;
		}

		int Manager::SetupOffenseSquad(const std::string& Candidate_Name) {
			int team = (int)bots.at(Candidate_Name).JoinedTeam() - 1;
			assert(team == 0 || team == 1);

			auto& squad = squads[team];
			squad.emplace_back(Squad(Candidate_Name, 3, Policy::Offense));
			return squad.size() - 1;
		}

		int Manager::SetupPlayerSquad() {
			int team = (int)common::GetTeamFromModel(game::game.GetHost()) - 1;
			assert(team == 0 || team == 1);

			auto& squad = squads[team];
			squad.emplace_back(Squad(STRING(game::game.GetHost()->v.netname), 5, Policy::Player));
			return squad.size() - 1;
		}

		int Manager::JoinSquad(const std::string& Name, Policy Will_Policy) {
			for (auto& squad : squads) {
				for (int i = 0; i < squad.size(); i++) {
				if (squad[i].GetPolicy() == Will_Policy && squad[i].Join(Name)) {
					return i;
				}
			}
			}
			return -1;
		}

		std::shared_ptr<game::Client> Manager::GetSquadLeader(const common::Team team, const int Squad_Index) {
			return game::game.clients.Get(squads[(int)team - 1][Squad_Index].LeaderName());
		}

		void Manager::LeftSquad(const std::string& Name) {
			auto& squad = squads[(int)bots.at(Name).JoinedTeam() - 1];
			for (int i = 0; i < squad.size(); i++) {
				if (squad[i].Left(Name))
					break;
			}
		}

		bool Manager::IsBotLeader(const std::string& Name, const int Index) const noexcept {
			auto& squad = squads[(int)bots.at(Name).JoinedTeam() - 1];
			return squad[Index].LeaderName() == Name;
		}

		float Memory::Evaluate(const int Index) {
			// In this function, evaluate whether the memory is valuable.

			auto& memory = memories[Index];
			float evaluation{};
			evaluation = gpGlobals->time - memory.time_occurrence;
			return evaluation;
		}

		void Memory::Memorize(const Vector& Source, EventType type, float time) {
			// Memorize a new event.

			memories.emplace_back(Source, type, time, -1);
			for (int i = memories.size() - 2; i >= 0; i--) {
				if (type == memories[i].type) {
					memories[memories.size() - 1].parent = i;
					break;
				}
			}
		}

		void Memory::Clear() {
			memories.clear();
		}

		Bot::Bot(std::shared_ptr<game::Client> assigned_client, const common::Team Join_Team, const common::Model Select_Model) POKEBOT_DEBUG_NOEXCEPT :
			client(assigned_client)
		{
			team = Join_Team;
			model = Select_Model;

			OnNewRound();
		}

		Squad::Squad(const std::string& Leader_Name, const size_t Number_Limit, const Policy Initial_Policy) : leader_name(Leader_Name), limit(Number_Limit), policy(Initial_Policy) {
			members.insert(Leader_Name);
		}

		bool Squad::Join(const std::string& Member_Name) {
			if (members.size() >= limit || members.find(Member_Name) != members.end()) {
				return false;
			}
			members.insert(Member_Name);
			return true;
		}

		bool Squad::Left(const std::string& Member_Name) {
			return members.erase(Member_Name) > 0;
		}
	}
}