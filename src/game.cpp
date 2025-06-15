#include "bot/manager.hpp"
#include <unordered_set>


import pokebot.game;
import pokebot.game.util;
import pokebot.game.client;
import pokebot.util;
import pokebot.util.tracer;

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

		bool Hostage::RecoginzeOwner(const std::string_view& Client_Name) POKEBOT_NOEXCEPT {
			auto client = game.clients.Get(Client_Name.data());
			if (client != nullptr && game::Distance(client->origin, entity->v.origin) < 83.0f && client->GetTeam() == game::Team::CT) {
				if (owner_name.c_str() == Client_Name) {
					owner_name.clear();
				} else {
					owner_name = Client_Name.data();
				}
				return true;
			}
			return false;
		}
		
		game::Team Client::GetTeam() const noexcept { return game::GetTeamFromModel(client); }
		bool Client::IsFakeClient() const noexcept{ return bool(flags & util::Third_Party_Bot_Flag); }

		void Hostage::Update() POKEBOT_NOEXCEPT {
			if (owner_name.empty())
				return;
			
			auto owner = game.clients.Get(owner_name.data());
			const bool Is_Owner_Terrorist = owner->GetTeam() == game::Team::T;
			if (IsReleased() || owner->GetTeam() == game::Team::T || game::Distance(owner->origin, entity->v.origin) > 200.0f)
				owner_name.clear();
		}

		bool Hostage::IsUsed() const POKEBOT_NOEXCEPT { return game.PlayerExists(owner_name.data()); }
		bool Hostage::IsOwnedBy(const std::string_view& Name) const POKEBOT_NOEXCEPT { return (IsUsed() && owner_name.data() == Name); }
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
			bot::Manager::Instance().OnNewRoundPreparation();
			clients.OnNewRound();
#if false
			node::world.OnNewRound();
#else
			node::czworld.OnNewRound();
#endif

			auto getNumber = [this](const char* class_name) -> size_t {
				size_t number{};
				edict_t* entity = nullptr;
				while ((entity = game::FindEntityByClassname(entity, class_name)) != nullptr) {
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
				if (bool(client.second.button & IN_ATTACK)) {
					produced_sound = Sound{ .origin = Vector{}, .volume = 100 };
				}

				if (bool(client.second.button & IN_USE)) {
					// Sound occurs.
					produced_sound = Sound{ .origin = client.second.origin, .volume = 50 };
					// Recoginze the player as a owner.
					for (auto& hostage : hostages) {
						if (hostage.RecoginzeOwner(client.first.data())) {
							break;
						}
					}
				}
			}
		}

		void Game::PostUpdate() noexcept {
			for (auto& client : clients.GetAll()) {
				if (client.second.IsFakeClient()) {
					const_cast<Client&>(client.second).ResetKey();
				}
			}
		}


		void Game::Init(edict_t* entities, int max) {
			map_flags = static_cast<MapFlags>(0);
			hostages.clear();

			for (int i = 0; i < max; ++i) {
				auto ent = entities + i;
				if (const std::string_view classname = STRING(ent->v.classname); classname == "info_player_start" || classname == "info_vip_start") {
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
		
		bool Game::Kill(const std::string_view& Client_Name) {
			if (PlayerExists(Client_Name.data())) {
				MDLL_ClientKill(const_cast<edict_t*>(game::game.clients.Get(Client_Name.data())->Edict()));
				return true;
			} else {
				return false;
			}
		}

		size_t Game::GetNumberOfHostages() const noexcept {
			return hostages.size();
		}

		size_t Game::GetNumberOfLivingHostages() const noexcept {
			auto&& living_hostages = (hostages | std::views::filter([](const Hostage& Target) -> bool { return static_cast<const edict_t*>(Target)->v.health > 0; }));
			return std::distance(living_hostages.begin(), living_hostages.end());
		}
		
		size_t Game::GetNumberOfRescuedHostages() const noexcept {
			auto&& living_hostages = (hostages | std::views::filter([](const Hostage& Target) -> bool { return bool(static_cast<const edict_t*>(Target)->v.effects & EF_NODRAW); }));
			return std::distance(living_hostages.begin(), living_hostages.end());
		}

		std::optional<Vector> Game::GetHostageOrigin(const int Index) const noexcept {
			if (Index < 0 || Index >= hostages.size())
				return std::nullopt;

			return hostages[Index].Origin();
		}

		bool Game::IsHostageUsed(const int Index) const POKEBOT_NOEXCEPT {
			return hostages[Index].IsUsed();
		}

		bool Game::IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name) {
			return hostages[Index].IsOwnedBy(Owner_Name);
		}

		const edict_t* const Game::GetClosedHostage(const Vector& Origin, const float Base_Distance) {
			for (auto& hostage : hostages) {
				if (game::Distance(hostage.Origin(), Origin) <= Base_Distance) {
					return hostage;
				}
			}
			return nullptr;
		}

		const char* const Game::GetBotArg(const size_t Index) const POKEBOT_NOEXCEPT {
			if (bot_args.size() == Index) {
				return bot_args.at(Index - 1).c_str();
			} else {
				return bot_args.at(Index).c_str();
			}
		}

		size_t Game::GetBotArgCount() const POKEBOT_NOEXCEPT {
			return bot_args.size();
		}

		bool Game::IsBotCmd() const POKEBOT_NOEXCEPT {
			return !bot_args.empty();
		}

		size_t Game::GetLives(const game::Team) const POKEBOT_NOEXCEPT {
			return 0;
		}

		uint32_t Game::CurrentRonud() const POKEBOT_NOEXCEPT {
			return round;
		}
		
		bool Game::IsCurrentMode(const MapFlags Game_Mode) const POKEBOT_NOEXCEPT {
			return bool(map_flags & Game_Mode);
		}

		MapFlags Game::GetScenario() const POKEBOT_NOEXCEPT {
			return map_flags;
		}

		void Game::IssueCommand(const std::string_view& Client_Name, util::fixed_string<32u> sentence) POKEBOT_NOEXCEPT {
			bot_args.clear();
			char* arg = strtok(sentence.data(), " ");
			bot_args.push_back(arg);
			while ((arg = strtok(nullptr, " ")) != nullptr) {
				bot_args.push_back(arg);
			}
			MDLL_ClientCommand(const_cast<edict_t*>(clients.Get(Client_Name.data())->Edict()));
			bot_args.clear();
		}

		bool Client::HasHostages() const POKEBOT_NOEXCEPT {
			for (int i = 0; i < game::game.GetNumberOfHostages(); i++) {
				if (game::game.IsHostageOwnedBy(i, Name())) {
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

		void Client::PressKey(const int Key) noexcept {
			if (bool(Key & IN_USE)) {
				use_reset_time = gpGlobals->time + 1.0f;
			}
			client->v.button |= Key;
		}

		void Client::ResetKey() noexcept {
			if (use_reset_time > gpGlobals->time) {
				client->v.button &= IN_USE;
			} else {
				client->v.button = 0;
			}
		}

		void ClientManager::OnNewRound() {
			for (auto& client : clients) {
				client.second.is_nvg_on = false;
			}
		}

		ClientCreationResult ClientManager::Create(std::string_view client_name) {
			assert(!client_name.empty());
			if (client_name.empty())
				return std::make_tuple(false, "");

			auto client = (*g_engfuncs.pfnCreateFakeClient)(client_name.data());
			if (client == nullptr)
				return std::make_tuple(false, "");

			client_name = STRING(client->v.netname);
			if (client->pvPrivateData != nullptr)
				FREE_PRIVATE(client);

			client->pvPrivateData = nullptr;
			client->v.frags = 0;

			// END OF FIX: --- score resetted
			CALL_GAME_ENTITY(PLID, "player", VARS(client));
			client::ClientKey client_key{ client };
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
			if (!MDLL_ClientConnect(client, client_name.data(), "127.0.0.1", ptr))
				return std::make_tuple(false, "");

			MDLL_ClientPutInServer(client);
			client->v.flags |= pokebot::util::Third_Party_Bot_Flag;
			return std::make_tuple(Register(client), client_name.data());
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
				if (target->health - Health <= 0) {
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

		void ClientManager::OnTeamAssigned(const std::string_view Client_Name, const game::Team Assigned_Team) POKEBOT_NOEXCEPT {
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

		bool Client::CanSeeEntity(const edict_t* const Target) const noexcept {
			return entity::CanSeeEntity(Edict(), Target);
		}

		bool Client::CanSeeFriend() const POKEBOT_NOEXCEPT {
			for (auto& other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*this, other.second) && other.second.GetTeam() == GetTeam()) {
					return true;
				}
			}
			return false;
		}

		void Client::GetEnemyNamesWithinView(pokebot::util::PlayerName player_names[32]) const POKEBOT_NOEXCEPT {
			int i = 0;
			for (const auto& other : game.clients.GetAll()) {
				if (other.second.IsDead() || other.second.GetTeam() == GetTeam()) {
					continue;
				}

				if (entity::CanSeeEntity(*this, other.second)) {
					player_names[i++] = other.first.data();
				}
			}
		}

		void Client::GetEntityNamesInView(pokebot::util::PlayerName player_names[32]) const POKEBOT_NOEXCEPT {
			int i = 0;
			for (auto& other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*this, other.second)) {
					player_names[i++] = other.first.data();
				}
			}
		}

		
		game::Team Client::GetTeamFromModel() const POKEBOT_NOEXCEPT {
			return game::GetTeamFromModel(client);
		}

		bool Client::IsPlayerModelReloading() const POKEBOT_NOEXCEPT {
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
				this->sequence == sequence[static_cast<int>(CurrentWeapon()) - 1][0] || 
				this->sequence == sequence[static_cast<int>(CurrentWeapon()) - 1][1];
			return (Is_Model_Reloading);
		}

		bool Client::IsViewModelReloading() const POKEBOT_NOEXCEPT {
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
				this->weaponanim == weaponanim[static_cast<int>(CurrentWeapon()) - 1][0] ||
				this->weaponanim == weaponanim[static_cast<int>(CurrentWeapon()) - 1][1];

			return (Is_View_Reloading);
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
}