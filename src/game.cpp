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
			if (common::Distance(client->origin, entity->v.origin) < 83.0f && client->GetTeam() == common::Team::CT) {
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
			if (IsReleased() || owner->GetTeam() == common::Team::T || common::Distance(owner->origin, entity->v.origin) > 200.0f)
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
			node::world.OnNewRound();
		}

		void Game::Update() {
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
					produced_sound = Sound{ .origin = client.second->origin, .volume = 50 };
					// Recoginze the player as a owner.
					for (auto& hostage : hostages) {
						if (hostage.RecoginzeOwner(client.second)) {
							break;
						}
					}
				}

				if (bool(static_cast<edict_t*>(*client.second)->v.flags & pokebot::common::Third_Party_Bot_Flag)) {
					static_cast<edict_t*>(*client.second)->v.flags |= pokebot::common::Third_Party_Bot_Flag;
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
					pokebot::node::world.Add(pokebot::game::game.host.Origin(), pokebot::node::GoalKind::None);
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
			return std::make_shared<FakeClient>(client);
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
			if (decltype(auto) target = Get(Client_Name.data()); target->Health - Health <= 0) {
				OnDeath(Client_Name);
			} else {
				// TODO: Send the event message for a bot.
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
			Get(Client_Name.data())->weapon[static_cast<int>(Weapon_ID) - 1].clip = Amount;
		}

		void ClientManager::OnAmmoPickedup(const std::string_view Client_Name, const game::Weapon Weapon_ID, const int Amount) noexcept {
			Get(Client_Name.data())->weapon[static_cast<int>(Weapon_ID) - 1].ammo = Amount;
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

		bool ClientStatus::CanSeeEnemy() const noexcept {
			for (auto other : game.clients.GetAll()) {
				if (entity::CanSeeEntity(*client, *other.second) && other.second->GetTeam() != GetTeam()) {
					return true;
				}
			}
			return false;
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