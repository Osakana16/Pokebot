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

		Hostage Hostage::AttachHostage(const edict_t* Hostage_Entity) noexcept {
			assert(Hostage_Entity != nullptr);
			Hostage hostage{};
			hostage.entity = Hostage_Entity;
			hostage.owner = nullptr;
			return hostage;
		}

		bool Hostage::RecoginzeOwner(std::shared_ptr<Client>& client) noexcept {
			if (common::Distance(client->origin(), entity->v.origin) < 83.0f && client->GetTeam() == common::Team::CT) {
				if (owner == client) {
					owner = nullptr;
				} else {
					owner = client;
				}
				return true;
			}
			return false;
		}

		void Hostage::Update() noexcept {
			if (owner == nullptr)
				return;

			const bool Is_Owner_Terrorist = owner->GetTeam() == common::Team::T;
			if (IsReleased() || owner->GetTeam() == common::Team::T || common::Distance(owner->origin(), entity->v.origin) > 200.0f)
				owner = nullptr;
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

		void Game::OnNewRound() noexcept {
			round++;
			bot::manager.OnNewRound();
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
			
			for (auto client : clients.GetAll()) {
				Sound produced_sound{};
				if (bool(client.second->Button() & IN_ATTACK)) {
					produced_sound = Sound{ .origin = Vector{}, .volume = 100 };
				}

				if (bool(client.second->Button() & IN_USE)) {
					// Sound occurs.
					produced_sound = Sound{ .origin = client.second->origin(), .volume = 50 };
					// Recoginze the player as a owner.
					for (auto& hostage : hostages) {
						if (hostage.RecoginzeOwner(client.second)) {
							break;
						}
					}
				}
			}
		}

		void Game::PostUpdate() {
			for (auto client : clients.GetAll()) {
				if (bool(static_cast<edict_t*>(*client.second)->v.flags & pokebot::common::Third_Party_Bot_Flag)) {
					static_cast<edict_t*>(*client.second)->v.button = 0;
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

		size_t Game::GetHostageNumber() const noexcept {
			return hostages.size();
		}

		bool Game::IsHostageUsed(const int Index) const noexcept {
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

		const std::string& Game::GetBotArg(const size_t Index) const noexcept {
			return bot_args[Index];
		}

		size_t Game::GetBotArgCount() const noexcept {
			return bot_args.size();
		}

		bool Game::IsBotCmd() const noexcept {
			return !bot_args.empty();
		}

		size_t Game::GetLives(const common::Team) const noexcept {
			return 0;
		}

		uint32_t Game::CurrentRonud() const noexcept {
			return round;
		}
		
		bool Game::IsCurrentMode(const MapFlags Game_Mode) const noexcept {
			return bool(map_flags & Game_Mode);
		}

		MapFlags Game::GetMapFlag() const noexcept {
			return map_flags;
		}

		void Game::IssueCommand(edict_t* client, const std::string& Sentence) noexcept {
			bot_args.clear();
			bot_args = common::StringSplit(&Sentence, ' ');
			MDLL_ClientCommand(client);
			bot_args.clear();
		}

		bool Client::IsPlayerModelReloading() const noexcept {
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
				client->v.sequence == sequence[static_cast<int>(current_weapon) - 1][0] || 
				client->v.sequence == sequence[static_cast<int>(current_weapon) - 1][1];
			return (Is_Model_Reloading);
		}

		bool Client::IsViewModelReloading() const noexcept {
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
				client->v.weaponanim == weaponanim[static_cast<int>(current_weapon) - 1][0] ||
				client->v.weaponanim == weaponanim[static_cast<int>(current_weapon) - 1][1];

			return (Is_View_Reloading);
		}

		bool Client::HasHostages() const noexcept {
			for (int i = 0; i < game::game.GetHostageNumber(); i++) {
				if (game::game.IsHostageOwnedBy(i, Name())) {
					return true;
				}
			}
			return false;
		}

		bool Host::IsHostValid() const noexcept {
			return host != nullptr;
		}

		const char* const Host::HostName() const noexcept { return STRING(host->v.netname); }
		const Vector& Host::Origin() const noexcept { return host->v.origin; }

		void Host::SetHost(edict_t* const target) noexcept {
			host = target;
		}
		
		void Host::Update() {
			if (host != nullptr) {
				host->v.health = 255;
				if (game::is_enabled_auto_waypoint &&  (host->v.deadflag != DEAD_DEAD && host->v.deadflag != DEAD_DYING && host->v.movetype != MOVETYPE_NOCLIP)) {
#if !USE_NAVMESH
					pokebot::node::world.Add(pokebot::game::game.host.Origin(), pokebot::node::GoalKind::None);
#endif
				}
			}
		}

		std::shared_ptr<Client> Client::Create(std::string client_name) {
			DEBUG_PRINTF("{}\n", __FUNCTION__);
			if (client_name.empty())
				return nullptr;

			auto client = (*g_engfuncs.pfnCreateFakeClient)(client_name.c_str());
			if (client == nullptr)
				return nullptr;

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
				return nullptr;

			MDLL_ClientPutInServer(client);
			client->v.flags |= pokebot::common::Third_Party_Bot_Flag;
			return std::make_shared<Client>(client);
		}

		std::shared_ptr<Client> Client::Attach(edict_t* edict) {
			return std::make_shared<Client>(edict);
		}


		std::shared_ptr<Client> Client::Attach(const int Index) {
			return std::make_shared<Client>(INDEXENT(Index));
		}

		void ClientManager::OnNewRound() {
			if (vip != nullptr) vip->is_vip = false;
			vip = nullptr;
			for (auto& client : clients) {
				client.second->is_nvg_on = false;
			}
		}

		std::shared_ptr<Client> ClientManager::Create(std::string client_name) {
			assert(!client_name.empty());
			auto client = Client::Create(client_name);
			client_name = client->Name();
			return clients.insert({ client_name, client }).first->second;
		}

		std::shared_ptr<Client> ClientManager::Register(edict_t* edict) {
			return clients.insert({ STRING(edict->v.netname), Client::Attach(edict) }).first->second;
		}

		void ClientManager::OnDeath(const std::string_view Client_Name) noexcept {
			decltype(auto) target = Get(Client_Name.data());
			target->status_icon = StatusIcon::Not_Displayed;
			target->item = Item::None;
		}

		void ClientManager::OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) noexcept {
			if (auto target = Get(Client_Name.data()); target != nullptr) {
				if (target->Health() - Health <= 0) {
					OnDeath(Client_Name);
				} else {
					// TODO: Send the event message for a bot.
				}
			}
		}

		void ClientManager::OnMoneyChanged(const std::string_view Client_Name, const int Money) noexcept {
			Get(Client_Name.data())->money = Money;
		}

		void ClientManager::OnScreenFaded(const std::string_view Client_Name) noexcept {

		}

		void ClientManager::OnNVGToggled(const std::string_view Client_Name, const bool Toggle) noexcept {
			Get(Client_Name.data())->is_nvg_on = Toggle;
		}

		void ClientManager::OnWeaponChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID) noexcept {
			Get(Client_Name.data())->current_weapon = Weapon_ID;
		}

		void ClientManager::OnClipChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID, const int Amount) noexcept {
			Get(Client_Name.data())->weapon_clip = Amount;
		}

		void ClientManager::OnAmmoPickedup(const std::string_view Client_Name, const game::AmmoID Ammo_ID, const int Amount) noexcept {
			Get(Client_Name.data())->weapon_ammo[static_cast<int>(Ammo_ID)] = Amount;
		}

		void ClientManager::OnTeamAssigned(const std::string_view Client_Name, const common::Team Assigned_Team) noexcept {
			auto target = Get(Client_Name.data());
			if (target != nullptr)
				target->team = Assigned_Team;
		}

		void ClientManager::OnItemChanged(const std::string_view Client_Name, game::Item item) noexcept {
			Get(Client_Name.data())->item |= item;
		}

		void ClientManager::OnStatusIconShown(const std::string_view Client_Name, const StatusIcon Icon) noexcept {
			Get(Client_Name.data())->status_icon |= Icon;
		}

		void ClientManager::OnVIPChanged(const std::string_view Client_Name) noexcept {
			auto&& candidate = Get(Client_Name.data());
			assert(vip == candidate || vip == nullptr);
			vip = candidate;
			vip->is_vip = true;
		}

		void ClientManager::OnDefuseKitEquiped(const std::string_view Client_Name) noexcept {
			Get(Client_Name.data())->item |= Item::Defuse_Kit;
		}

		ClientStatus ClientManager::GetClientStatus(std::string_view client_name) {
			return ClientStatus{ Get(client_name.data()) };
		}

		ClientStatus::ClientStatus(const std::shared_ptr<Client>& target) : client(target) {}

		common::Team ClientStatus::GetTeam() const noexcept {
			return client->GetTeam();
		}

		bool ClientStatus::CanSeeFriend() const noexcept {
			for (auto other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*client, *other.second) && other.second->GetTeam() == GetTeam()) {
					return true;
				}
			}
			return false;
		}

		std::shared_ptr<Client> ClientStatus::GetEnemyWithinView() const noexcept {
			for (auto other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*client, *other.second) && other.second->GetTeam() != GetTeam()) {
					return other.second;
				}
			}
			return nullptr;
		}

		std::vector<const edict_t*> ClientStatus::GetEntitiesInView() const noexcept {
			decltype(GetEntitiesInView()) result{};
			for (auto other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*client, *other.second)) {
					result.push_back(*other.second);
				}
			}
			return result;
		}
	}

	namespace entity {
		bool InViewCone(edict_t* const self, const Vector& Origin) noexcept {
			MAKE_VECTORS(self->v.angles);
			const auto Vector_2D_Los = (Origin - self->v.origin).Make2D().Normalize();
			const auto Dot = DotProduct(Vector_2D_Los, gpGlobals->v_forward.Make2D());
			return (Dot > 0.50);
		}

		bool IsVisible(edict_t* const self, const Vector& Origin) noexcept {
			// look through caller's eyes
			TraceResult tr;
			UTIL_TraceLine(self->v.origin + self->v.view_ofs, Origin, dont_ignore_monsters, ignore_glass, self, &tr);
			return (tr.flFraction >= 1.0);	// line of sight is not established or valid
		}

		bool CanSeeEntity(edict_t* const self, const const edict_t* Target) noexcept {
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
		ClientKey::ClientKey(edict_t* target) noexcept :
			Client_Index(ENTINDEX(target)),
			infobuffer((*g_engfuncs.pfnGetInfoKeyBuffer)(target)) {
		}

		ClientKey& ClientKey::SetValue(const char* Key, const char* Value) noexcept {
			(*g_engfuncs.pfnSetClientKeyValue)(Client_Index, infobuffer, Key, Value);
			return *this;
		}
	}
}