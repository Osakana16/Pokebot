export module pokebot.game: game_manager;
import :cs_game_manager;

import pokebot.database;
import pokebot.game.client;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.game.entity;
import pokebot.util;

export namespace pokebot::game {
	constexpr float Default_Max_Move_Speed = 255.0f;

	// variable type
	enum class Var {
		Normal = 0,
		ReadOnly,
		Password,
		NoServer,
		GameRef
	};

	class Host {
		edict_t* host{};
	public:
		void ShowMenu();

		const edict_t* AsEdict() const POKEBOT_NOEXCEPT { return host; }
		bool IsHostValid() const POKEBOT_NOEXCEPT;
		void SetHost(edict_t* const target) POKEBOT_NOEXCEPT;
		const char* const HostName() const POKEBOT_NOEXCEPT;
		const Vector& Origin() const POKEBOT_NOEXCEPT;
		void Update();
	};

	// ConVar class from YapBot © Copyright YaPB Project Developers
	// 
	// simplify access for console variables
	class ConVar final {
	public:
		cvar_t* ptr;

		ConVar() = delete;
		~ConVar() = default;

		ConVar(const char* name, const char* initval, Var type = Var::NoServer, bool regMissing = false, const char* regVal = nullptr);
		ConVar(const char* name, const char* initval, const char* info, bool bounded = true, float min = 0.0f, float max = 1.0f, Var type = Var::NoServer, bool regMissing = false, const char* regVal = nullptr);

		explicit operator bool() const POKEBOT_NOEXCEPT { return ptr->value > 0.0f; }
		explicit operator int() const POKEBOT_NOEXCEPT { return static_cast<int>(ptr->value); }
		explicit operator float() const POKEBOT_NOEXCEPT { return ptr->value; }
		explicit operator const char* () const POKEBOT_NOEXCEPT { return ptr->string; }

		void operator=(const float val) POKEBOT_NOEXCEPT { g_engfuncs.pfnCVarSetFloat(ptr->name, val); }
		void operator=(const int val) POKEBOT_NOEXCEPT { operator=(static_cast<float>(val)); }
		void operator=(const char* val) POKEBOT_NOEXCEPT { g_engfuncs.pfnCvar_DirectSet(ptr, const_cast<char*>(val)); }

	};

	struct ConVarReg {
		cvar_t reg;
		util::fixed_string<64u> info;
		util::fixed_string<64u> init;
		const char* regval;
		class ConVar* self;
		float initial, min, max;
		bool missing;
		bool bounded;
		Var type;
	};

	class Hostage final {
		Hostage() = default;
		Hostage(const Hostage&);
		Hostage& operator=(const Hostage&) = delete;

		util::Time time{};

		const edict_t* entity;
		pokebot::util::PlayerName owner_name{};
	public:
		operator const edict_t* const () const POKEBOT_NOEXCEPT {
			return entity;
		}

		static Hostage AttachHostage(const edict_t* Hostage_Entity) POKEBOT_NOEXCEPT {
			assert(Hostage_Entity != nullptr);
			Hostage hostage{};
			hostage.entity = Hostage_Entity;
			hostage.owner_name.clear();
			return hostage;
		}

		bool RecoginzeOwner(const std::string_view& Client_Name) POKEBOT_NOEXCEPT;

		void Update() POKEBOT_NOEXCEPT;
		bool IsUsed() const POKEBOT_NOEXCEPT;
		bool IsOwnedBy(const std::string_view& Name) const POKEBOT_NOEXCEPT { return (IsUsed() && owner_name.data() == Name); }
		bool IsReleased() const POKEBOT_NOEXCEPT { return (entity->v.effects & EF_NODRAW); }
		const Vector& Origin() const POKEBOT_NOEXCEPT {
			return entity->v.origin;
		}

		Hostage(Hostage&& h) POKEBOT_NOEXCEPT {
			owner_name = std::move(h.owner_name);
			assert(h.owner_name.empty());
			entity = h.entity;
			h.entity = nullptr;
		}
	};

	class Game : public CSGameBase {
		database::Database database{};
		std::vector<Hostage> hostages{};

		std::vector<util::fixed_string<32u>> bot_args{};
		MapFlags map_flags{};
		uint32_t round{};
		bool is_newround{};

		std::vector<ConVarReg> convars{};
		std::optional<Vector> c4_origin{};
		edict_t* backpack{};

	public:
		std::optional<Vector> GetC4Origin() const noexcept {
			return c4_origin;
		}

		std::optional<Vector> GetBackpackOrigin() const noexcept {
			return backpack != nullptr ? std::make_optional(backpack->v.origin) : std::nullopt;
		}

		Host host{};
		client::ClientManager clients{};

		bool RegisterClient(edict_t* client) { return clients.Register(client); }

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
#if 0
			if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
				if (!c4_origin.has_value()) {
					if (bomber_name.empty()) {
						// When the bomb is dropped:
						// 
						if (backpack != nullptr) {

						} else {
							edict_t* dropped_bomb{};
							while ((dropped_bomb = game::FindEntityByClassname(dropped_bomb, "weaponbox")) != NULL) {
								if (std::string_view(STRING(dropped_bomb->v.model)) == "models/w_backpack.mdl") {
									backpack = dropped_bomb;
									break;
								}
							}
						}
					}

					edict_t* c4{};
					while ((c4 = game::FindEntityByClassname(c4, "grenade")) != nullptr) {
						if (std::string_view(STRING(c4->v.model)) == "models/w_c4.mdl") {
							c4_origin = c4->v.origin;
							break;
						}
					}

					if (c4_origin.has_value()) {
						// OnBombPlanted();
					}
				}
			}

			for (auto& hostage : hostages) {
				hostage.Update();
			}
#endif
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

		void OnWeaponChanged(const std::string_view Client_Name, const weapon::ID Weapon_ID) POKEBOT_NOEXCEPT {
			clients.OnWeaponChanged(Client_Name, Weapon_ID);
		}

		void OnClipChanged(const std::string_view Client_Name, const weapon::ID Weapon_ID, const int Clip) POKEBOT_NOEXCEPT {
			clients.OnClipChanged(Client_Name, Weapon_ID, Clip);
		}

		void OnAmmoPickedup(const std::string_view Client_Name, const weapon::ammo::ID Ammo_ID, const int Ammo) POKEBOT_NOEXCEPT {
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


	ConVar::ConVar(const char* name, const char* initval, Var type, bool regMissing, const char* regVal) {
		game.AddCvar(name, initval, "", false, 0.0f, 0.0f, type, regMissing, regVal, this);
	}

	ConVar::ConVar(const char* name, const char* initval, const char* info, bool bounded, float min, float max, Var type, bool regMissing, const char* regVal) {
		game.AddCvar(name, initval, info, bounded, min, max, type, regMissing, regVal, this);
	}


	ConVar poke_freeze{ "pk_freeze", "0" };
	ConVar poke_fight{ "pk_fight", "1" };
	ConVar poke_buy{ "pk_buy", "1" };


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

	void Hostage::Update() POKEBOT_NOEXCEPT {
		if (owner_name.empty())
			return;

		auto owner = game.clients.Get(owner_name.data());
		const bool Is_Owner_Terrorist = owner->GetTeam() == game::Team::T;
		if (IsReleased() || owner->GetTeam() == game::Team::T || game::Distance(owner->origin, entity->v.origin) > 200.0f)
			owner_name.clear();
	}

	bool Hostage::IsUsed() const POKEBOT_NOEXCEPT { return game.PlayerExists(owner_name.data()); }
}