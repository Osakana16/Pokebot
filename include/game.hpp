#pragma once
#include "database.hpp"
namespace pokebot {
	namespace game {
		using ClientCreationResult = std::tuple<bool, common::PlayerName>;

		inline bool is_enabled_auto_waypoint = true;

		POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
			StatusIcon,
			Not_Displayed,
			Buy_Zone = 1 << 0,
			Defuser = 1 << 1,
			C4 = 1 << 2,
			Rescue_Zone = 1 << 3,
			Vip_Safety = 1 << 4,
			Escape_Zone = 1 << 5
		);

		POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
			Item,
			None,
			Nightvision=1<<0,
			Defuse_Kit=1<<1
		);

		enum class Weapon {
			None = -1,
			P228 = 1,
			Shield = 2,
			Scout = 3,
			HEGrenade = 4,
			XM1014 = 5,
			C4 = 6,
			MAC10 = 7,
			AUG = 8,
			Smoke = 9,
			Elite = 10,
			FiveSeven = 11,
			UMP45 = 12,
			SG550 = 13,
			Galil = 14,
			Famas = 15,
			USP = 16,
			Glock18 = 17,
			AWP = 18,
			MP5 = 19,
			M249 = 20,
			M3 = 21,
			M4A1 = 22,
			TMP = 23,
			G3SG1 = 24,
			Flashbang = 25,
			Deagle = 26,
			SG552 = 27,
			AK47 = 28,
			Knife = 29,
			P90 = 30,
			Armor = 31,
			ArmorHelm = 32,
			Defuser = 33
		};

		enum class AmmoID {
			None,
			Magnum338,
			Nato776,
			NatoBox556,
			Nato556,
			Buckshot,
			ACP45,
			MM57,
			AE50,
			SIG357,
			MM9
		};

		constexpr float Default_Max_Move_Speed = 255.0f;

		constexpr int Primary_Weapon_Bit = (common::ToBit<int>(Weapon::M3) | common::ToBit<int>(Weapon::XM1014) | common::ToBit<int>(Weapon::MAC10) | common::ToBit<int>(Weapon::TMP) | common::ToBit<int>(Weapon::MP5) | common::ToBit<int>(Weapon::UMP45) | common::ToBit<int>(Weapon::P90) | common::ToBit<int>(Weapon::Famas) | common::ToBit<int>(Weapon::Galil) | common::ToBit<int>(Weapon::AK47) | common::ToBit<int>(Weapon::M4A1) | common::ToBit<int>(Weapon::AUG) | common::ToBit<int>(Weapon::SG552) | common::ToBit<int>(Weapon::SG550) | common::ToBit<int>(Weapon::G3SG1) | common::ToBit<int>(Weapon::Scout) | common::ToBit<int>(Weapon::AWP) | common::ToBit<int>(Weapon::M249));
		constexpr int Secondary_Weapon_Bit = (common::ToBit<int>(Weapon::P228) | common::ToBit<int>(Weapon::USP) | common::ToBit<int>(Weapon::Deagle) | common::ToBit<int>(Weapon::FiveSeven) | common::ToBit<int>(Weapon::Glock18) | common::ToBit<int>(Weapon::Elite));
		constexpr int Melee_Bit = (common::ToBit<int>(Weapon::Knife));
		constexpr int Grenade_Bit = (common::ToBit<int>(Weapon::HEGrenade) | common::ToBit<int>(Weapon::Flashbang) | common::ToBit<int>(Weapon::Smoke));
		constexpr int C4_Bit = (common::ToBit<int>(Weapon::C4));

		enum class WeaponType {
			Secondary,
			Primary,
			Melee,
			Grenade,
			Special
		};

		constexpr WeaponType Weapon_Type[31]{
			WeaponType::Special,
			WeaponType::Secondary,//1
			WeaponType::Secondary,//2
			WeaponType::Primary,//3
			WeaponType::Grenade,//4
			WeaponType::Primary,//5
			WeaponType::Special,//6
			WeaponType::Primary,//7
			WeaponType::Primary,//8
			WeaponType::Grenade,//9
			WeaponType::Secondary,//10
			WeaponType::Secondary,//11
			WeaponType::Primary,//12
			WeaponType::Primary,//13
			WeaponType::Primary,//14
			WeaponType::Primary,//15
			WeaponType::Secondary,//16
			WeaponType::Secondary,//17
			WeaponType::Primary,//18
			WeaponType::Primary,//19
			WeaponType::Primary,//20
			WeaponType::Primary,//21
			WeaponType::Primary,//22
			WeaponType::Primary,//23
			WeaponType::Primary,//24
			WeaponType::Grenade,//25
			WeaponType::Secondary,//26
			WeaponType::Primary,//27
			WeaponType::Primary,//28
			WeaponType::Melee,//29
			WeaponType::Primary//30
		};

		using WeaponName = const char* const;
		constexpr std::tuple<WeaponName, AmmoID> Weapon_CVT[30]{
			std::make_tuple<WeaponName, AmmoID>("weapon_p228", AmmoID::SIG357),
			std::make_tuple<WeaponName, AmmoID>("weapon_shield", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_scout", AmmoID::Nato776),
			std::make_tuple<WeaponName, AmmoID>("weapon_hegrenade", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_xm1014", AmmoID::Buckshot),
			std::make_tuple<WeaponName, AmmoID>("weapon_c4", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_aug", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_mac10", AmmoID::ACP45),
			std::make_tuple<WeaponName, AmmoID>("weapon_smoke", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_elite", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_fiveseven", AmmoID::MM57),
			std::make_tuple<WeaponName, AmmoID>("weapon_ump45", AmmoID::ACP45),
			std::make_tuple<WeaponName, AmmoID>("weapon_sg550", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_galil", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_famas", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_usp", AmmoID::ACP45),
			std::make_tuple<WeaponName, AmmoID>("weapon_glock18", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_awp", AmmoID::Magnum338),
			std::make_tuple<WeaponName, AmmoID>("weapon_mp5navy", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_m249", AmmoID::NatoBox556),
			std::make_tuple<WeaponName, AmmoID>("weapon_m3", AmmoID::Buckshot),
			std::make_tuple<WeaponName, AmmoID>("weapon_m4a1", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_tmp", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_g3sg1", AmmoID::Nato776),
			std::make_tuple<WeaponName, AmmoID>("weapon_flashbang", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_deagle", AmmoID::AE50),
			std::make_tuple<WeaponName, AmmoID>("weapon_sg552", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_ak47", AmmoID::Nato776),
			std::make_tuple<WeaponName, AmmoID>("weapon_knife", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_p90", AmmoID::MM57)
		};

		// variable type
		enum class Var {
			Normal = 0,
			ReadOnly,
			Password,
			NoServer,
			GameRef
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
			explicit operator const char* () const POKEBOT_NOEXCEPT { return ptr->string;  }
		
			void operator=(const float val) POKEBOT_NOEXCEPT { g_engfuncs.pfnCVarSetFloat(ptr->name, val); }
			void operator=(const int val) POKEBOT_NOEXCEPT { operator=(static_cast<float>(val)); }
			void operator=(const char* val) POKEBOT_NOEXCEPT { g_engfuncs.pfnCvar_DirectSet(ptr, const_cast<char*>(val)); }

		};

		extern ConVar poke_freeze;
		extern ConVar poke_fight;
		extern ConVar poke_buy;

		struct ConVarReg {
			cvar_t reg;
			common::fixed_string<64u> info;
			common::fixed_string<64u> init;
			const char* regval;
			class ConVar* self;
			float initial, min, max;
			bool missing;
			bool bounded;
			Var type;
		};

		class Client {
			friend class ClientManager;

			edict_t* const client{};

			common::Team team{};
			int money{};
			StatusIcon status_icon{};
			Item item{};
			bool is_nvg_on{};
			bool is_vip{};

			common::Time use_reset_time{};

			common::Array<int, 10> weapon_ammo{};
			int weapon_clip{};
			Weapon current_weapon{};
		public:
			static bool IsDead(const edict_t* const Target) POKEBOT_NOEXCEPT { return Target->v.deadflag == DEAD_DEAD || Target->v.health <= 0 || Target->v.movetype == MOVETYPE_NOCLIP; }
			static bool IsValid(const edict_t* const Target) POKEBOT_NOEXCEPT { return (Target != nullptr && !Target->free); }

			Client() = delete;
			Client(const Client&) = delete;
			Client& operator=(const Client&) = delete;
		public:
            Client(edict_t* e) POKEBOT_NOEXCEPT : 
				client(e),
				health(client->v.health),
				armor(client->v.armorvalue),
				max_health(client->v.max_health),
				speed(client->v.maxspeed),
				Money(money),
				view_ofs(client->v.view_ofs),
				origin(client->v.origin),
				angles(client->v.angles),
				avelocity(client->v.avelocity),
				punchangle(client->v.punchangle),
				v_angle(client->v.v_angle),
				ideal_yaw(client->v.ideal_yaw),
				idealpitch(client->v.idealpitch),
				flags(client->v.flags),
				velocity(client->v.velocity),
				movetype(client->v.movetype),
				weapons(client->v.weapons),
				sequence(client->v.sequence),
				button(client->v.button),
				impulse(client->v.impulse),
				weaponanim(client->v.weaponanim),
				index(ENTINDEX(const_cast<edict_t*>(e)))
            {}

			const int& index;
			const float& health;
			const float& armor;
			const float& max_health;
			const float& speed;
			const int& Money;
			const Vector& origin;

			int& button;
			int& impulse;
			Vector& view_ofs;
			Vector& angles;
			Vector& avelocity;
			Vector& punchangle;
			Vector& v_angle;
			float& ideal_yaw;
			float& idealpitch;
			int& flags;
			int& movetype;
			int& weapons;
			int& weaponanim;
			int& sequence;
			const Vector& velocity;

			StatusIcon DisplayingStatusIcon() const POKEBOT_NOEXCEPT { return status_icon; }
			Weapon CurrentWeapon() const POKEBOT_NOEXCEPT { return current_weapon; }
			int CurrentWeaponClip() const POKEBOT_NOEXCEPT { return weapon_clip; }

			edict_t* Edict() POKEBOT_NOEXCEPT { return client; }
			const edict_t* Edict() const POKEBOT_NOEXCEPT { return client; }
			operator edict_t* () POKEBOT_NOEXCEPT { return Edict(); }
			operator const edict_t* () const POKEBOT_NOEXCEPT { return client; }

			const char* const ClassName() const POKEBOT_NOEXCEPT { return STRING(client->v.classname); }
			const char* const Name() const POKEBOT_NOEXCEPT { return STRING(client->v.netname); }

			void PressKey(const int Key) noexcept;
			void ResetKey() noexcept;

			common::Team GetTeam() const POKEBOT_NOEXCEPT { return common::GetTeamFromModel(client); }

			bool IsValid() const POKEBOT_NOEXCEPT { return IsValid(client); }
			bool IsDead() const POKEBOT_NOEXCEPT { return IsDead(client); }

			bool IsVIP() const POKEBOT_NOEXCEPT { return is_vip; }
			int WeaponAmmo(const AmmoID Ammo_ID) const POKEBOT_NOEXCEPT { return weapon_ammo[static_cast<int>(Ammo_ID) - 1]; }

			bool IsInBuyzone() const POKEBOT_NOEXCEPT { return bool(DisplayingStatusIcon() & StatusIcon::Buy_Zone); }
			bool IsInEscapezone() const POKEBOT_NOEXCEPT { return bool(DisplayingStatusIcon() & StatusIcon::Escape_Zone); }
			bool IsInRescuezone() const POKEBOT_NOEXCEPT { return bool(DisplayingStatusIcon() & StatusIcon::Rescue_Zone); }
			bool IsInVipSafety() const POKEBOT_NOEXCEPT { return bool(DisplayingStatusIcon() & StatusIcon::Vip_Safety); }
			bool HasDefuser() const POKEBOT_NOEXCEPT { return bool(DisplayingStatusIcon() & StatusIcon::Defuser); }

			bool IsWalking() const noexcept { return bool(velocity.Length2D() <= 150.0f); }
			bool IsDucking() const POKEBOT_NOEXCEPT { return bool(button & IN_DUCK); }
			bool IsInWater() const POKEBOT_NOEXCEPT { return bool(flags & FL_INWATER); }
			bool IsOnFloor() const POKEBOT_NOEXCEPT { return bool(flags & (FL_ONGROUND | FL_PARTIALGROUND)) != 0; }
			bool IsOnTrain() const POKEBOT_NOEXCEPT { return bool(flags & FL_ONTRAIN); }
			bool IsFiring() const POKEBOT_NOEXCEPT { return bool(button & IN_ATTACK); }
			bool IsReadyToThrowGrenade() const POKEBOT_NOEXCEPT { return IsFiring() && bool(weapons & Grenade_Bit); }
			bool IsPlantingBomb() const POKEBOT_NOEXCEPT { return IsFiring() && bool(weapons & C4_Bit) && (sequence == 63 || sequence == 61); }
			bool IsClimblingLadder() const POKEBOT_NOEXCEPT { return (movetype & MOVETYPE_FLY); }

			bool HasHostages() const POKEBOT_NOEXCEPT;
			bool HasWeapon(const Weapon Weapon_ID) const POKEBOT_NOEXCEPT { return bool(weapons & common::ToBit<int>(Weapon_ID)); }
			bool HasPrimaryWeapon() const POKEBOT_NOEXCEPT { return weapons & game::Primary_Weapon_Bit; }
			bool HasSecondaryWeapon() const POKEBOT_NOEXCEPT { return weapons & game::Secondary_Weapon_Bit; }

			/**
			* @brief Check the clip of current weapon remains or not
			* @return 
			*/
			bool IsOutOfClip() const POKEBOT_NOEXCEPT { return (Weapon_Type[static_cast<int>(CurrentWeapon())] == WeaponType::Primary || Weapon_Type[static_cast<int>(CurrentWeapon())] == WeaponType::Secondary) && CurrentWeaponClip() <= 0; }
			bool IsOutOfAmmo(const AmmoID Ammo_ID) const POKEBOT_NOEXCEPT { return WeaponAmmo(Ammo_ID) <= 0; }
			bool IsOutOfCurrentWeaponAmmo() const POKEBOT_NOEXCEPT { 
				return 
					(Weapon_Type[static_cast<int>(CurrentWeapon())] == WeaponType::Primary || 
					 Weapon_Type[static_cast<int>(CurrentWeapon())] == WeaponType::Secondary) && 
					IsOutOfAmmo(std::get<AmmoID>(Weapon_CVT[static_cast<int>(CurrentWeapon()) - 1])); 
			}

			bool IsViewModelReloading() const POKEBOT_NOEXCEPT;
			bool IsPlayerModelReloading() const POKEBOT_NOEXCEPT;

			bool CanSeeEntity(const edict_t*) const noexcept;
			bool CanSeeFriend() const POKEBOT_NOEXCEPT;
			void GetEnemyNamesWithinView(common::PlayerName player_names[32]) const POKEBOT_NOEXCEPT;
			void GetEntityNamesInView(common::PlayerName player_names[32]) const POKEBOT_NOEXCEPT;
			common::Team GetTeamFromModel() const POKEBOT_NOEXCEPT;

			bool IsFakeClient() const POKEBOT_NOEXCEPT { return bool(flags & common::Third_Party_Bot_Flag); }
		};

		class ClientManager {
			std::unordered_map<common::PlayerName, Client, common::PlayerName::Hash> clients{};
		public:
			void OnNewRound();
			ClientCreationResult Create(std::string_view client_name);
			bool Register(edict_t*);
			auto& GetAll() const POKEBOT_NOEXCEPT {
				return clients;
			}

			const Client* Get(const char* const Name) const POKEBOT_NOEXCEPT {
				if (auto it = clients.find(Name); it != clients.end()) {
					return &it->second;
				}
				return nullptr;
			}
						
			Client* GetAsMutable(const char* const Name) POKEBOT_NOEXCEPT {
				if (auto it = clients.find(Name); it != clients.end()) {
					return &it->second;
				}
				return nullptr;
			}

			bool Disconnect(const char* const Name) noexcept {
				return clients.erase(Name) > 0;
			}

			void OnVIPChanged(const std::string_view Client_Name) POKEBOT_NOEXCEPT;
			void OnDefuseKitEquiped(const std::string_view Client_Name) POKEBOT_NOEXCEPT;
			void OnDeath(const std::string_view Client_Name) POKEBOT_NOEXCEPT;
			void OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) POKEBOT_NOEXCEPT;
			void OnMoneyChanged(const std::string_view Client_Name, const int) POKEBOT_NOEXCEPT;
			void OnScreenFaded(const std::string_view Client_Name) POKEBOT_NOEXCEPT;
			void OnNVGToggled(const std::string_view Client_Name, const bool) POKEBOT_NOEXCEPT;
			void OnWeaponChanged(const std::string_view Client_Name, const game::Weapon) POKEBOT_NOEXCEPT;
			void OnClipChanged(const std::string_view Client_Name, const game::Weapon, const int) POKEBOT_NOEXCEPT;
			void OnAmmoPickedup(const std::string_view Client_Name, const game::AmmoID, const int) POKEBOT_NOEXCEPT;
			void OnTeamAssigned(const std::string_view Client_Name, common::Team) POKEBOT_NOEXCEPT;
			void OnItemChanged(const std::string_view Client_Name, game::Item) POKEBOT_NOEXCEPT;
			void OnStatusIconShown(const std::string_view Client_Name, const StatusIcon) POKEBOT_NOEXCEPT;
		};

		struct Sound final {
			Vector origin{};
			int volume{};
		};

		POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
			MapFlags,
			Demolition = 1 << 0,
			HostageRescue = 1 << 1,
			Assassination = 1 << 2,			
			Escape = 1 << 3
		);

		class Hostage final {
			Hostage() = default;
			Hostage(const Hostage&);
			Hostage& operator=(const Hostage&) = delete;

			common::Time time{};

			const edict_t* entity;
			common::PlayerName owner_name{};
		public:
			operator const edict_t* const () const POKEBOT_NOEXCEPT {
				return entity;
			}
			void Update() POKEBOT_NOEXCEPT;
			bool RecoginzeOwner(const std::string_view&) POKEBOT_NOEXCEPT;

			bool IsUsed() const POKEBOT_NOEXCEPT;
			bool IsOwnedBy(const std::string_view& Name) const POKEBOT_NOEXCEPT;
			bool IsReleased() const POKEBOT_NOEXCEPT;
			static Hostage AttachHostage(const edict_t*) POKEBOT_NOEXCEPT;
			const Vector& Origin() const POKEBOT_NOEXCEPT;

			Hostage(Hostage&& h) POKEBOT_NOEXCEPT {
				owner_name = std::move(h.owner_name);
				assert(h.owner_name.empty());
				entity = h.entity;
				h.entity = nullptr;
			}
		};

		class Host {
			edict_t* host{};
		public:
			const edict_t* AsEdict() const POKEBOT_NOEXCEPT { return host; }
			bool IsHostValid() const POKEBOT_NOEXCEPT;
			void SetHost(edict_t* const target) POKEBOT_NOEXCEPT;
			const char* const HostName() const POKEBOT_NOEXCEPT;
			const Vector& Origin() const POKEBOT_NOEXCEPT;
			void Update();
		};

		inline class Game {
			database::Database database{};
			std::vector<Hostage> hostages{};

			std::vector<common::fixed_string<32u>> bot_args{};
			MapFlags map_flags{};
			uint32_t round{};
			bool is_newround{};

			std::vector<ConVarReg> convars{};
		public:
			ClientManager clients{};

			bool Kill(const std::string_view&);

			ClientCreationResult Spawn(const std::string_view& client_name) { return clients.Create(client_name.data()); }
			bool RegisterClient(edict_t* client) { return clients.Register(client); }

			Host host{};

			// - Hostage -

			/**
			* @brief Get the number of hostages.
			* @return Number of hostages.
			*/
			size_t GetNumberOfHostages() const noexcept;

			/**
			* @brief Get the number of hostages who are not dead.
			* @return Number of living hostages.
			*/
			size_t GetNumberOfLivingHostages() const noexcept;
			
			/**
			* @brief Get the number of hostages who are not dead.
			* @return Number of living hostages.
			*/
			size_t GetNumberOfRescuedHostages() const noexcept;

			/**
			* @brief Get the hostage origin.
			* @param Index The hostage index
			* @return The origin of a hostage. If the specified hostage doesn't exist, return nullopt.
			*/
			std::optional<Vector> GetHostageOrigin(const int Index) const noexcept;

			/**
			* @brief Check the hostage is used or not.
			* @param Index The hostage index
			* @return True if the hostage is leading by a player, false if it's not.
			*/
			bool IsHostageUsed(const int Index) const POKEBOT_NOEXCEPT;

			/**
			* @brief Check the hostage is used by the specified player.
			* @param Index The hostage index
			* @param Owner_Name The player name
			* @return True if the hostage owned by the specified player, false if it's not.
			*/
			bool IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name);
			const edict_t* const GetClosedHostage(const Vector& Origin, const float Base_Distance);

			const char* const GetBotArg(const size_t Index) const POKEBOT_NOEXCEPT;
			size_t GetBotArgCount() const POKEBOT_NOEXCEPT;
			bool IsBotCmd() const POKEBOT_NOEXCEPT;

			size_t GetLives(const common::Team) const POKEBOT_NOEXCEPT;	// Get the number of lives of the team.
			uint32_t CurrentRonud() const POKEBOT_NOEXCEPT;
			bool IsCurrentMode(const MapFlags) const POKEBOT_NOEXCEPT;
			MapFlags GetMapFlag() const POKEBOT_NOEXCEPT;
			void IssueCommand(const std::string_view&, common::fixed_string<32u> sentence) POKEBOT_NOEXCEPT;

			void Init(edict_t* entities, int max);
			void PreUpdate();
			void PostUpdate() noexcept;

			bool PlayerExists(const char* const Client_Name) const POKEBOT_NOEXCEPT { return clients.Get(Client_Name) != nullptr; }

			void AddCvar(const char *name, const char *value, const char *info, bool bounded, float min, float max, Var varType, bool missingAction, const char *regval, ConVar *self);
			void RegisterCvars();

			// - Event funcs -

			void OnNewRound() POKEBOT_NOEXCEPT;

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

			void OnWeaponChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID) POKEBOT_NOEXCEPT {
				clients.OnWeaponChanged(Client_Name, Weapon_ID);
			}

			void OnClipChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID, const int Clip) POKEBOT_NOEXCEPT {
				clients.OnClipChanged(Client_Name, Weapon_ID, Clip);
			}

			void OnAmmoPickedup(const std::string_view Client_Name, const game::AmmoID Ammo_ID, const int Ammo) POKEBOT_NOEXCEPT {
				clients.OnAmmoPickedup(Client_Name, Ammo_ID, Ammo);
			}

			void OnTeamAssigned(const std::string_view Client_Name, common::Team Team) POKEBOT_NOEXCEPT {
				clients.OnTeamAssigned(Client_Name, Team);
			}

			void OnItemChanged(const std::string_view Client_Name, game::Item Item_ID) POKEBOT_NOEXCEPT {
				clients.OnItemChanged(Client_Name, Item_ID);
			}

			void OnStatusIconShown(const std::string_view Client_Name, const StatusIcon Icon) POKEBOT_NOEXCEPT {
				clients.OnStatusIconShown(Client_Name, Icon);
			}

		} game{};
	}

	namespace entity {
		bool CanSeeEntity(const edict_t* const self, const const edict_t* Target) POKEBOT_NOEXCEPT;
		bool InViewCone(const edict_t* const self, const Vector& Origin) POKEBOT_NOEXCEPT;
		bool IsVisible(const edict_t* const Self, const Vector& Origin) POKEBOT_NOEXCEPT;
	}

	namespace engine {
		class ClientKey final {
			const int Client_Index{};
			char* const infobuffer{};
		public:
			ClientKey(edict_t* target) POKEBOT_NOEXCEPT;
			ClientKey& SetValue(const char* Key, const char* Value) POKEBOT_NOEXCEPT;
		};
	}
}