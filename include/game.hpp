#pragma once
#include "database.hpp"
#include "util/timer.hpp"
#include "game/team.hpp"
#include "game/map.hpp"

namespace pokebot {
	namespace game {
		using ClientCreationResult = std::tuple<bool, pokebot::util::PlayerName>;

		inline bool is_enabled_auto_waypoint = true;

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

		constexpr int Primary_Weapon_Bit = (game::ToBit<int>(Weapon::M3) | game::ToBit<int>(Weapon::XM1014) | game::ToBit<int>(Weapon::MAC10) | game::ToBit<int>(Weapon::TMP) | game::ToBit<int>(Weapon::MP5) | game::ToBit<int>(Weapon::UMP45) | game::ToBit<int>(Weapon::P90) | game::ToBit<int>(Weapon::Famas) | game::ToBit<int>(Weapon::Galil) | game::ToBit<int>(Weapon::AK47) | game::ToBit<int>(Weapon::M4A1) | game::ToBit<int>(Weapon::AUG) | game::ToBit<int>(Weapon::SG552) | game::ToBit<int>(Weapon::SG550) | game::ToBit<int>(Weapon::G3SG1) | game::ToBit<int>(Weapon::Scout) | game::ToBit<int>(Weapon::AWP) | game::ToBit<int>(Weapon::M249));
		constexpr int Secondary_Weapon_Bit = (game::ToBit<int>(Weapon::P228) | game::ToBit<int>(Weapon::USP) | game::ToBit<int>(Weapon::Deagle) | game::ToBit<int>(Weapon::FiveSeven) | game::ToBit<int>(Weapon::Glock18) | game::ToBit<int>(Weapon::Elite));
		constexpr int Melee_Bit = (game::ToBit<int>(Weapon::Knife));
		constexpr int Grenade_Bit = (game::ToBit<int>(Weapon::HEGrenade) | game::ToBit<int>(Weapon::Flashbang) | game::ToBit<int>(Weapon::Smoke));
		constexpr int C4_Bit = (game::ToBit<int>(Weapon::C4));

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
			util::fixed_string<64u> info;
			util::fixed_string<64u> init;
			const char* regval;
			class ConVar* self;
			float initial, min, max;
			bool missing;
			bool bounded;
			Var type;
		};


		struct Sound final {
			Vector origin{};
			int volume{};
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
			void ShowMenu();

			const edict_t* AsEdict() const POKEBOT_NOEXCEPT { return host; }
			bool IsHostValid() const POKEBOT_NOEXCEPT;
			void SetHost(edict_t* const target) POKEBOT_NOEXCEPT;
			const char* const HostName() const POKEBOT_NOEXCEPT;
			const Vector& Origin() const POKEBOT_NOEXCEPT;
			void Update();
		};

	}

	namespace entity {
		bool CanSeeEntity(const edict_t* const self, const edict_t* const Target) POKEBOT_NOEXCEPT;
		bool InViewCone(const edict_t* const self, const Vector& Origin) POKEBOT_NOEXCEPT;
		bool IsVisible(const edict_t* const Self, const Vector& Origin) POKEBOT_NOEXCEPT;
	}
}