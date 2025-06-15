export module pokebot.game: game_manager;
import :cs_game_manager;

import pokebot.game.client;
import pokebot.game.util;

export namespace pokebot::game {
	class Game : public CSGameBase {
		database::Database database{};
		std::vector<Hostage> hostages{};

		std::vector<util::fixed_string<32u>> bot_args{};
		MapFlags map_flags{};
		uint32_t round{};
		bool is_newround{};

		std::vector<ConVarReg> convars{};
	public:
		client::ClientManager clients{};


		ClientCreationResult Spawn(const std::string_view& client_name) { return clients.Create(client_name.data()); }
		bool RegisterClient(edict_t* client) { return clients.Register(client); }

		Host host{};

		void AddCvar(const char* name, const char* value, const char* info, bool bounded, float min, float max, Var varType, bool missingAction, const char* regval, ConVar* self) {
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

		void RegisterCvars() {
			for (auto& var : convars) {
				ConVar& self = *var.self;
				cvar_t& reg = var.reg;
				self.ptr = g_engfuncs.pfnCVarGetPointer(reg.name);

				if (!self.ptr) {
					g_engfuncs.pfnCVarRegister(&var.reg);
					self.ptr = g_engfuncs.pfnCVarGetPointer(reg.name);
				}
			}
		}

		void OnNewRound() noexcept {
			round++;
			clients.OnNewRound();

			auto getNumber = [this](const char* class_name) -> size_t {
				size_t number{};
				edict_t* entity = nullptr;
				while ((entity = FindEntityByClassname(entity, class_name)) != nullptr) {
					number++;
				}
				return number;
			};
		}

		void PreUpdate() {
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

		void PostUpdate() noexcept {
			for (auto& client : clients.GetAll()) {
				if (client.second.IsFakeClient()) {
					const_cast<client::Client&>(client.second).ResetKey();
				}
			}
		}

		void Init(edict_t* entities, int max) {
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
#
		bool Kill(const std::string_view& Client_Name) {
			if (PlayerExists(Client_Name.data())) {
				MDLL_ClientKill(const_cast<edict_t*>(clients.Get(Client_Name.data())->Edict()));
				return true;
			} else {
				return false;
			}
		}

		size_t GetNumberOfHostages() const noexcept {
			return hostages.size();
		}

		size_t GetNumberOfLivingHostages() const noexcept {
			auto&& living_hostages = (hostages | std::views::filter([](const Hostage& Target) -> bool { return static_cast<const edict_t*>(Target)->v.health > 0; }));
			return std::distance(living_hostages.begin(), living_hostages.end());
		}

		size_t GetNumberOfRescuedHostages() const noexcept {
			auto&& living_hostages = (hostages | std::views::filter([](const Hostage& Target) -> bool { return bool(static_cast<const edict_t*>(Target)->v.effects & EF_NODRAW); }));
			return std::distance(living_hostages.begin(), living_hostages.end());
		}

		std::optional<Vector> GetHostageOrigin(const int Index) const noexcept {
			if (Index < 0 || Index >= hostages.size())
				return std::nullopt;

			return hostages[Index].Origin();
		}

		bool PlayerExists(const char* const Client_Name) const POKEBOT_NOEXCEPT { return clients.Get(Client_Name) != nullptr; }

		bool IsHostageUsed(const int Index) const POKEBOT_NOEXCEPT {
			return hostages[Index].IsUsed();
		}

		bool IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name) {
			return hostages[Index].IsOwnedBy(Owner_Name);
		}

		const edict_t* const GetClosedHostage(const Vector& Origin, const float Base_Distance) {
			for (auto& hostage : hostages) {
				if (Distance(hostage.Origin(), Origin) <= Base_Distance) {
					return hostage;
				}
			}
			return nullptr;
		}

		const char* const GetBotArg(const size_t Index) const POKEBOT_NOEXCEPT {
			if (bot_args.size() == Index) {
				return bot_args.at(Index - 1).c_str();
			} else {
				return bot_args.at(Index).c_str();
			}
		}

		size_t GetBotArgCount() const POKEBOT_NOEXCEPT {
			return bot_args.size();
		}

		bool IsBotCmd() const POKEBOT_NOEXCEPT {
			return !bot_args.empty();
		}

		size_t GetLives(const Team) const POKEBOT_NOEXCEPT {
			return 0;
		}

		uint32_t CurrentRonud() const POKEBOT_NOEXCEPT {
			return round;
		}

		bool IsCurrentMode(const MapFlags Game_Mode) const POKEBOT_NOEXCEPT {
			return bool(map_flags & Game_Mode);
		}

		MapFlags GetScenario() const POKEBOT_NOEXCEPT {
			return map_flags;
		}

		void IssueCommand(const std::string_view& Client_Name, util::fixed_string<32u> sentence) POKEBOT_NOEXCEPT {
			bot_args.clear();
			char* arg = strtok(sentence.data(), " ");
			bot_args.push_back(arg);
			while ((arg = strtok(nullptr, " ")) != nullptr) {
				bot_args.push_back(arg);
			}
			MDLL_ClientCommand(const_cast<edict_t*>(clients.Get(Client_Name.data())->Edict()));
			bot_args.clear();
		}

		void OnVIPChanged(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			clients.OnVIPChanged(Client_Name);
		}

		void OnDefuseKitEquiped(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			clients.OnDefuseKitEquiped(Client_Name);
		}

		void OnDeath(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			clients.OnDeath(Client_Name);
		}

		void OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) POKEBOT_NOEXCEPT {
			clients.OnDamageTaken(Client_Name, Inflictor, Health, Armor, Bit);
		}

		void OnMoneyChanged(const std::string_view Client_Name, const int Money) POKEBOT_NOEXCEPT {
			clients.OnMoneyChanged(Client_Name, Money);
		}

		void OnScreenFaded(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			clients.OnScreenFaded(Client_Name);
		}

		void OnNVGToggled(const std::string_view Client_Name, const bool Toggled) POKEBOT_NOEXCEPT {
			clients.OnNVGToggled(Client_Name, Toggled);
		}

		void OnWeaponChanged(const std::string_view Client_Name, const Weapon Weapon_ID) POKEBOT_NOEXCEPT {
			clients.OnWeaponChanged(Client_Name, Weapon_ID);
		}

		void OnClipChanged(const std::string_view Client_Name, const Weapon Weapon_ID, const int Clip) POKEBOT_NOEXCEPT {
			clients.OnClipChanged(Client_Name, Weapon_ID, Clip);
		}

		void OnAmmoPickedup(const std::string_view Client_Name, const AmmoID Ammo_ID, const int Ammo) POKEBOT_NOEXCEPT {
			clients.OnAmmoPickedup(Client_Name, Ammo_ID, Ammo);
		}

		void OnTeamAssigned(const std::string_view Client_Name, Team Team) POKEBOT_NOEXCEPT {
			clients.OnTeamAssigned(Client_Name, Team);
		}

		void OnItemChanged(const std::string_view Client_Name, Item Item_ID) POKEBOT_NOEXCEPT {
			clients.OnItemChanged(Client_Name, Item_ID);
		}

		void OnStatusIconShown(const std::string_view Client_Name, const StatusIcon Icon) POKEBOT_NOEXCEPT {
			clients.OnStatusIconShown(Client_Name, Icon);
		}
	};

	Game game{};
}