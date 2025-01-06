#include <unordered_set>

namespace pokebot {
	namespace game {
		ConVar::ConVar(const char* name, const char* initval, Var type, bool regMissing, const char* regVal) {
			game.AddCvar(name, initval, "", false, 0.0f, 0.0f, type, regMissing, regVal, this);
		}
		
		ConVar::ConVar(const char* name, const char* initval, const char* info, bool bounded, float min, float max, Var type, bool regMissing, const char* regVal) {
			game.AddCvar(name, initval, info, bounded, min, max, type, regMissing, regVal, this);
		}

		ConVar poke_freeze{ "pk_freeze", "0" };
		ConVar poke_fight{ "pk_fight", "1"};
		ConVar poke_buy{ "pk_buy", "1"};

		Hostage Hostage::AttachHostage(const edict_t* Hostage_Entity) POKEBOT_NOEXCEPT {
			assert(Hostage_Entity != nullptr);
			Hostage hostage{};
			hostage.entity = Hostage_Entity;
			hostage.owner_name.clear();
			return hostage;
		}

		bool Hostage::RecoginzeOwner(const ClientName& Client_Name) POKEBOT_NOEXCEPT {
			auto client_status = game.GetClientStatus(Client_Name);
			if (common::Distance(client_status.origin(), entity->v.origin) < 83.0f && client_status.GetTeam() == common::Team::CT) {
				if (owner_name == Client_Name) {
					owner_name.clear();
				} else {
					owner_name = Client_Name;
				}
				return true;
			}
			return false;
		}

		void Hostage::Update() POKEBOT_NOEXCEPT {
			if (owner_name.empty())
				return;
			
			auto owner_status = game.GetClientStatus(owner_name);
			const bool Is_Owner_Terrorist = owner_status.GetTeam() == common::Team::T;
			if (IsReleased() || owner_status.GetTeam() == common::Team::T || common::Distance(owner_status.origin(), entity->v.origin) > 200.0f)
				owner_name.clear();
		}

		bool Hostage::IsUsed() const POKEBOT_NOEXCEPT { return game.PlayerExists(owner_name); }
		bool Hostage::IsOwnedBy(const std::string_view& Name) const POKEBOT_NOEXCEPT { return (IsUsed() && owner_name == Name); }
	 	bool Hostage::IsReleased() const POKEBOT_NOEXCEPT { return (entity->v.effects & EF_NODRAW); }
		const Vector& Hostage::Origin() const POKEBOT_NOEXCEPT {
			return entity->v.origin;
		}


		void Game::AddCvar(const char *name, const char *value, const char *info, bool bounded, float min, float max, Var varType, bool missingAction, const char *regval, ConVar *self) {
			ConVarReg reg{
				.reg = {
					.name = name,
					.string = value,
					.flags = FCVAR_EXTDLL
				},
				.info = info,
				.init = value,
				.regval = regval,
				.self = self,
				.initial = (float)std::atof(value),
				.min = min,
				.max = max,
				.missing = missingAction,
				.bounded = bounded,				
				.type = varType
			};

			switch (varType) {
				case Var::ReadOnly:
					reg.reg.flags |= FCVAR_SPONLY | FCVAR_PRINTABLEONLY;
					[[fallthrough]];
				case Var::Normal:
					reg.reg.flags |= FCVAR_SERVER;
					break;
				case Var::Password:
					reg.reg.flags |= FCVAR_PROTECTED;
					break;
			}
			convars.push_back(reg);
		}

		void Game::RegisterCvars() {
			for (auto& var : convars) {
				ConVar &self = *var.self;
				cvar_t &reg = var.reg;
				self.ptr = g_engfuncs.pfnCVarGetPointer (reg.name);

				if (!self.ptr) {
					g_engfuncs.pfnCVarRegister (&var.reg);
					self.ptr = g_engfuncs.pfnCVarGetPointer (reg.name);
				}
			}
		}

		void Game::OnNewRound() POKEBOT_NOEXCEPT {
			round++;
			bot::Manager::Instance().OnNewRound();
			clients.OnNewRound();
#if false
			node::world.OnNewRound();
#else
			node::czworld.OnNewRound();
#endif

			auto getNumber = [this](const char* class_name) -> size_t {
				size_t number{};
				edict_t* entity = nullptr;
				while ((entity = common::FindEntityByClassname(entity, class_name)) != nullptr) {
					number++;
				}
				return number;
			};
		}

		void Game::PreUpdate() {
			host.Update();

			for (auto& hostage : hostages) {
				hostage.Update();
			}
			
			for (const auto& client : clients.GetAll()) {
				Sound produced_sound{};
				if (bool(client.second.Button() & IN_ATTACK)) {
					produced_sound = Sound{ .origin = Vector{}, .volume = 100 };
				}

				if (bool(client.second.Button() & IN_USE)) {
					// Sound occurs.
					produced_sound = Sound{ .origin = client.second.origin(), .volume = 50 };
					// Recoginze the player as a owner.
					for (auto& hostage : hostages) {
						if (hostage.RecoginzeOwner(client.first)) {
							break;
						}
					}
				}
			}
		}

		void Game::PostUpdate() {
			for (const auto& client : clients.GetAll()) {
				if (clients.GetClientStatus(client.first).IsFakeClient()) {
					const_cast<edict_t*>(static_cast<const edict_t*>(client.second))->v.button = 0;
				}
			}
		}

		void Game::Init(edict_t* entities, int max) {
			hostages.clear();

			for (int i = 0; i < max; ++i) {
				auto ent = entities + i;
				std::string classname = STRING(ent->v.classname);
				if (classname == "info_player_start" || classname == "info_vip_start") {
					ent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
					ent->v.renderamt = 127; // set its transparency amount
					ent->v.effects |= EF_NODRAW;
				} else if (classname == "info_player_deathmatch") {
					ent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
					ent->v.renderamt = 127; // set its transparency amount
					ent->v.effects |= EF_NODRAW;
				} else if (classname == "func_vip_safetyzone" || classname == "info_vip_safetyzone") {
					map_flags |= MapFlags::Assassination; // assassination map
				} else if (classname == "hostage_entity") {
					hostages.push_back(std::move(Hostage::AttachHostage(ent)));
					map_flags |= MapFlags::HostageRescue; // rescue map
				} else if (classname == "func_bomb_target" || classname == "info_bomb_target") {
					map_flags |= MapFlags::Demolition; // defusion map
				} else if (classname == "func_escapezone") {
					map_flags |= MapFlags::Escape;

					// strange thing on some ES maps, where hostage entity present there
					if (bool(map_flags & MapFlags::HostageRescue)) {
						map_flags &= ~MapFlags::HostageRescue;
					}
				}
			}

			RegisterCvars();
		}
		
		void Game::RunPlayerMove(const ClientName& Client_Name, Vector movement_angle, float move_speed, float strafe_speed, float forward_speed, const std::uint8_t Msec_Value, const ClientCommitter& committer) {
			auto client = game::game.clients.GetAsMutable(Client_Name);
			client->PressKey(committer.button);
			client->angles() = committer.angles;
			client->v_angle() = committer.v_angle;
			client->idealpitch() = committer.idealpitch;
			client->ideal_yaw() = committer.idealyaw;

			client->flags() |= common::Third_Party_Bot_Flag;
			g_engfuncs.pfnRunPlayerMove(client->Edict(),
					movement_angle,
					move_speed,
					strafe_speed,
					0.0f,
					client->Button(),
					client->Impulse(),
					Msec_Value);
			client->Edict()->v.button = 0;
		}
		
		bool Game::Kill(const ClientName& Client_Name) {
			if (PlayerExists(Client_Name)) {
				MDLL_ClientKill(const_cast<edict_t*>(game::game.clients.Get(Client_Name)->Edict()));
				return true;
			} else {
				return false;
			}
		}

		size_t Game::GetHostageNumber() const POKEBOT_NOEXCEPT {
			return hostages.size();
		}

		bool Game::IsHostageUsed(const int Index) const POKEBOT_NOEXCEPT {
			return hostages[Index].IsUsed();
		}

		bool Game::IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name) {
			return hostages[Index].IsOwnedBy(Owner_Name);
		}

		const edict_t* const Game::GetClosedHostage(const Vector& Origin, const float Base_Distance) {
			for (auto& hostage : hostages) {
				if (common::Distance(hostage.Origin(), Origin) <= Base_Distance) {
					return hostage;
				}
			}
			return nullptr;
		}

		const std::string& Game::GetBotArg(const size_t Index) const POKEBOT_NOEXCEPT {
			return bot_args[Index];
		}

		size_t Game::GetBotArgCount() const POKEBOT_NOEXCEPT {
			return bot_args.size();
		}

		bool Game::IsBotCmd() const POKEBOT_NOEXCEPT {
			return !bot_args.empty();
		}

		size_t Game::GetLives(const common::Team) const POKEBOT_NOEXCEPT {
			return 0;
		}

		uint32_t Game::CurrentRonud() const POKEBOT_NOEXCEPT {
			return round;
		}
		
		bool Game::IsCurrentMode(const MapFlags Game_Mode) const POKEBOT_NOEXCEPT {
			return bool(map_flags & Game_Mode);
		}

		MapFlags Game::GetMapFlag() const POKEBOT_NOEXCEPT {
			return map_flags;
		}

		void Game::IssueCommand(const ClientName& Client_Name, const std::string& Sentence) POKEBOT_NOEXCEPT {
			bot_args.clear();
			bot_args = common::StringSplit(&Sentence, ' ');
			MDLL_ClientCommand(const_cast<edict_t*>(clients.Get(Client_Name)->Edict()));
			bot_args.clear();
		}

		bool ClientStatus::HasHostages() const POKEBOT_NOEXCEPT {
			for (int i = 0; i < game::game.GetHostageNumber(); i++) {
				if (game::game.IsHostageOwnedBy(i, client->Name())) {
					return true;
				}
			}
			return false;
		}

		bool Host::IsHostValid() const POKEBOT_NOEXCEPT {
			return host != nullptr;
		}

		const char* const Host::HostName() const POKEBOT_NOEXCEPT { return STRING(host->v.netname); }
		const Vector& Host::Origin() const POKEBOT_NOEXCEPT { return host->v.origin; }

		void Host::SetHost(edict_t* const target) POKEBOT_NOEXCEPT {
			host = target;
		}
		
		void Host::Update() {
			if (host != nullptr) {
#ifndef NDEBUG
				host->v.health = 255;
#endif
				if (game::is_enabled_auto_waypoint &&  (host->v.deadflag != DEAD_DEAD && host->v.deadflag != DEAD_DYING && host->v.movetype != MOVETYPE_NOCLIP)) {
#if !USE_NAVMESH
					pokebot::node::world.Add(pokebot::game::game.host.Origin(), pokebot::node::GoalKind::None);
#endif
				}
			}
		}

		
		void ClientCommitter::SetFakeClientFlag() {
			flags |= common::Third_Party_Bot_Flag;
		}

		void ClientCommitter::AddCommand(const std::string Command_Sentence) {
			commands.push_back(Command_Sentence);
		}

		void ClientCommitter::TurnViewAngle(common::AngleVector v) {
			v_angle = v;
			v_angle.z = 0.0f;
			angles.x = v_angle.x / 3;
			angles.y = v_angle.y;
			v_angle.x = -v_angle.x;
			angles.z = 0;
			
			idealyaw = v.y;
			if (idealyaw > 180.0f) {
				idealyaw -= 360.0f;
			} else if (idealyaw < -180.0f) {
				idealyaw += 360.0f;
			}
			
			idealpitch = v.x;
			if (idealpitch > 180.0f) {
				idealpitch-= 360.0f;
			} else if (idealpitch < -180.0f) {
				idealpitch += 360.0f;
			}
		}

		void ClientCommitter::PressKey(int key) {
			button |= key;
		}

		void ClientManager::OnNewRound() {
			for (auto& client : clients) {
				client.second.is_nvg_on = false;
			}
		}

		ClientCreationResult ClientManager::Create(std::string client_name) {
			assert(!client_name.empty());
			if (client_name.empty())
				return std::make_tuple(false, "");

			auto client = (*g_engfuncs.pfnCreateFakeClient)(client_name.c_str());
			if (client == nullptr)
				return std::make_tuple(false, "");

			client_name = STRING(client->v.netname);
			if (client->pvPrivateData != nullptr)
				FREE_PRIVATE(client);

			client->pvPrivateData = nullptr;
			client->v.frags = 0;

			// END OF FIX: --- score resetted
			CALL_GAME_ENTITY(PLID, "player", VARS(client));
			engine::ClientKey client_key{ client };
			client_key
				.SetValue("model", "")
				.SetValue("rate", "3500.000000")
				.SetValue("hud_fastswitch", "1")
				.SetValue("cl_updaterate", "20")
				.SetValue("tracker", "0")
				.SetValue("cl_dlmax", "128")
				.SetValue("lefthand", "1")
				.SetValue("friends", "0")
				.SetValue("dm", "0")
				.SetValue("ah", "1")
				.SetValue("_vgui_menus", "0");

			char ptr[128]{};            // allocate space for message from ClientConnect
			if (!MDLL_ClientConnect(client, client_name.c_str(), "127.0.0.1", ptr))
				return std::make_tuple(false, "");

			MDLL_ClientPutInServer(client);
			client->v.flags |= pokebot::common::Third_Party_Bot_Flag;
			return std::make_tuple(Register(client), client_name);
		}

		bool ClientManager::Register(edict_t* edict) {
			if (auto it = clients.find(STRING(edict->v.netname)); it == clients.end()) {
				clients.emplace(STRING(edict->v.netname), edict);
				return true;
			}
			return false;
		}

		void ClientManager::OnDeath(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			decltype(auto) target = GetAsMutable(Client_Name.data());
			target->status_icon = StatusIcon::Not_Displayed;
			target->item = Item::None;
		}

		void ClientManager::OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) POKEBOT_NOEXCEPT {
			if (auto target = GetAsMutable(Client_Name.data()); target != nullptr) {
				if (target->Health() - Health <= 0) {
					OnDeath(Client_Name);
				} else {
					// TODO: Send the event message for a bot.
				}
			}
		}

		void ClientManager::OnMoneyChanged(const std::string_view Client_Name, const int Money) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->money = Money;
		}

		void ClientManager::OnScreenFaded(const std::string_view Client_Name) POKEBOT_NOEXCEPT {

		}

		void ClientManager::OnNVGToggled(const std::string_view Client_Name, const bool Toggle) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->is_nvg_on = Toggle;
		}

		void ClientManager::OnWeaponChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->current_weapon = Weapon_ID;
		}

		void ClientManager::OnClipChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID, const int Amount) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->weapon_clip = Amount;
		}

		void ClientManager::OnAmmoPickedup(const std::string_view Client_Name, const game::AmmoID Ammo_ID, const int Amount) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->weapon_ammo[static_cast<int>(Ammo_ID)] = Amount;
		}

		void ClientManager::OnTeamAssigned(const std::string_view Client_Name, const common::Team Assigned_Team) POKEBOT_NOEXCEPT {
			auto target = GetAsMutable(Client_Name.data());
			if (target != nullptr)
				target->team = Assigned_Team;
		}

		void ClientManager::OnItemChanged(const std::string_view Client_Name, game::Item item) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->item |= item;
		}

		void ClientManager::OnStatusIconShown(const std::string_view Client_Name, const StatusIcon Icon) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->status_icon |= Icon;
		}

		void ClientManager::OnVIPChanged(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			auto&& candidate = GetAsMutable(Client_Name.data());
			candidate->is_vip = true;
		}

		void ClientManager::OnDefuseKitEquiped(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->item |= Item::Defuse_Kit;
		}

		ClientStatus ClientManager::GetClientStatus(std::string_view client_name) {
			return ClientStatus{ client_name.data() };
		}

		ClientStatus::ClientStatus(const ClientName& Client_Name) : client(game.clients.Get(Client_Name)) {}

		common::Team ClientStatus::GetTeam() const POKEBOT_NOEXCEPT {
			return client->GetTeam();
		}

		bool ClientStatus::CanSeeFriend() const POKEBOT_NOEXCEPT {
			for (auto& other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(client->Edict(), other.second) && other.second.GetTeam() == GetTeam()) {
					return true;
				}
			}
			return false;
		}

		std::vector<ClientName> ClientStatus::GetEnemyNamesWithinView() const POKEBOT_NOEXCEPT {
			decltype(GetEnemyNamesWithinView()) result{};
			for (const auto& other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*client, other.second) && other.second.GetTeam() != common::Team::Spector && !other.second.IsDead() && other.second.GetTeam() != GetTeam()) {
					result.push_back(other.first);
				}
			}
			return result;
		}

		std::vector<ClientName> ClientStatus::GetEntityNamesInView() const POKEBOT_NOEXCEPT {
			decltype(GetEntityNamesInView()) result{};
			for (auto& other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*client, other.second)) {
					result.push_back(other.first);
				}
			}
			return result;
		}

		
		common::Team ClientStatus::GetTeamFromModel() const POKEBOT_NOEXCEPT {
			return common::GetTeamFromModel(*client);
		}

		bool ClientStatus::IsPlayerModelReloading() const POKEBOT_NOEXCEPT {
			// The value of entvars_t::sequence.
			// sequence has two values. The value changes depending on whether the player is standing or not.
			// 
			// sequence[weapon_id][0] is the value when the player is standing.
			// sequence[weapon_id][1] is the value when the player is ducking.
			static int sequence[][2] {
				{ 21, 18 },	// P228
				{ 96, 93 },	// Shield
				{ 35, 32 },	// Scout
				{ -1, -1 },	// HEGrenade(No reload animation)
				{ 53, 50 },	// XM101
				{ -1, -1 },	// C4(No reload animation)
				{ 21, 21 },	// MAC10
				{ 15, 12 },	// Aug
				{ -1, -1 },	// Smoke(No reload animation)
				{ 29, 25 },	// Elite
				{ 21, 18 },	// Five-seveN
				{ 15, 12 },	// UMP45
				{ 35, 32 },	// SG550
				{ 82, 79 },	// Galil
				{ 15, 12 },	// Famas
				{ 21, 18 },	// USP(Without a silencer is 13, with silencer is 5)
				{ 21, 18 },	// Glock
				{ 35, 32 },	// AWP
				{ 41, 41 },	// MP5
				{ 53, 50 },	// M249
				{ 47, 44 },	// M3
				{ 35, 32 },	// M4A1
				{ 21, 18 },	// TMP
				{ 41, 38 },	// G3SG1
				{ 1, 1 },	// Flashbang
				{ 21, 18 },	// Deagle
				{ 41, 38 },	// SG552
				{ 82, 79 },	// AK47
				{ 1, 1 },	// Knife
				{ 15, 12 },	// P90
			};

			const bool Is_Model_Reloading = 
				client->Edict()->v.sequence == sequence[static_cast<int>(client->CurrentWeapon()) - 1][0] || 
				client->Edict()->v.sequence == sequence[static_cast<int>(client->CurrentWeapon()) - 1][1];
			return (Is_Model_Reloading);
		}

		bool ClientStatus::IsViewModelReloading() const POKEBOT_NOEXCEPT {
			// The value of entvars_t::weapon_anim.
			// weaponanim has two values, weapons with different animation values ​​are as follows:
			//	1. Glock(This is the only weapon the reload animation changes randomly).
			//  2. USP with or without silencer
			//	3. M4A1 with or without silencer
			//
			// weaponanim[weapon_id][0] is the value without silencer.
			// weaponanim[weapon_id][1] is the value with silencer.
			static int weaponanim[][2] {
				{ 5, 5 },	// P228
				{ 4, 4 },	// Shield	NOTE: Only shield has a problem; weaponanim never be changed after reloaded. This might causes something glitches.
				{ 3, 3 },	// Scout
				{ -1, -1 },	// HEGrenade(No reload animation)
				{ 5, 5 },	// XM1014(This weapon has multiple animations; 5 is the start of reloading, 3 is the just reloading, and 4 is the end of reloading).
				{ -1, -1 },	// C4(No reload animation)
				{ 1, 1 },	// MAC10
				{ 1, 1 },	// Aug
				{ -1, -1 },	// Smoke(No reload animation)
				{ 14, 14 },	// Elite
				{ 4, 4 },	// Five-seveN
				{ 1, 1 },	// UMP45
				{ 3, 3 },	// SG550
				{ 1, 1 },	// Galil
				{ 1, 1 },	// Famas
				{ 13, 5 },	// USP(Without a silencer is 13, with silencer is 5)
				{ 7, 12 },	// Glock(The one is 7, another is 12)
				{ 4, 4 },	// AWP
				{ 1, 1 },	// MP5
				{ 3, 3 },	// M249
				{ 5, 5 },	// M3(This weapon has multiple animations; 5 is the start of reloading, 3 is the just reloading, and 4 is the end of reloading).
				{ 11, 4 },	// M4A1(Without a silencer is 11, with silencer is 4)
				{ 1, 1 },	// TMP
				{ 3, 3 },	// G3SG1
				{ -1, -1 },	// Flashbang(No reload animation)
				{ 4, 4 },	// Deagle
				{ 1, 1 },	// SG552
				{ 1, 1 },	// AK47
				{ 1, 1 },	// Knife
				{ 1, 1 },	// P90
			};

			const bool Is_View_Reloading =
				client->Edict()->v.weaponanim == weaponanim[static_cast<int>(client->CurrentWeapon()) - 1][0] ||
				client->Edict()->v.weaponanim == weaponanim[static_cast<int>(client->CurrentWeapon()) - 1][1];

			return (Is_View_Reloading);
		}

		bool ClientStatus::IsFakeClient() const POKEBOT_NOEXCEPT {
			return bool(client->Edict()->v.flags & common::Third_Party_Bot_Flag);
		}

	}

	namespace entity {
		bool InViewCone(const edict_t* const self, const Vector& Origin) POKEBOT_NOEXCEPT {
			MAKE_VECTORS(self->v.angles);
			const auto Vector_2D_Los = (Origin - self->v.origin).Make2D().Normalize();
			const auto Dot = DotProduct(Vector_2D_Los, gpGlobals->v_forward.Make2D());
			return (Dot > 0.50);
		}

		bool IsVisible(const edict_t* const self, const Vector& Origin) POKEBOT_NOEXCEPT {
			// look through caller's eyes
			TraceResult tr;
			UTIL_TraceLine(self->v.origin + self->v.view_ofs, Origin, dont_ignore_monsters, ignore_glass, const_cast<edict_t*>(self), &tr);
			return (tr.flFraction >= 1.0);	// line of sight is not established or valid
		}

		bool CanSeeEntity(const edict_t* const self, const const edict_t* Target) POKEBOT_NOEXCEPT {
			const auto Body = Target->v.origin;
			const auto Head = Target->v.origin + Target->v.view_ofs;
			
			const bool Is_Body_In_ViewCone = InViewCone(self, Body);
			const bool Is_Body_Visible = IsVisible(self, Body);

			const bool Is_Head_In_ViewCone = InViewCone(self, Head);
			const bool Is_Head_Visible = IsVisible(self, Head);


			const bool Is_Body_In_FOV = Is_Body_Visible && Is_Body_In_ViewCone;
			const bool Is_Head_In_FOV = Is_Head_In_ViewCone && Is_Head_Visible;
			return (Is_Body_In_FOV || Is_Head_In_FOV);
		}
	}

	namespace engine {
		ClientKey::ClientKey(edict_t* target) POKEBOT_NOEXCEPT :
			Client_Index(ENTINDEX(target)),
			infobuffer((*g_engfuncs.pfnGetInfoKeyBuffer)(target)) {
		}

		ClientKey& ClientKey::SetValue(const char* Key, const char* Value) POKEBOT_NOEXCEPT {
			(*g_engfuncs.pfnSetClientKeyValue)(Client_Index, infobuffer, Key, Value);
			return *this;
		}
	}
}