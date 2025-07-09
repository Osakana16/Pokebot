export module pokebot.game.client: client;

import pokebot.game;
import pokebot.game.weapon;
import pokebot.game.player;
import pokebot.game.util;
import pokebot.util;

export namespace pokebot::game::client {
	class Client {
		friend class ClientManager;
	public:
		edict_t* const client{};

		game::Team team{};
		int money{};
		game::StatusIcon status_icon{};
		game::Item item{};
		bool is_nvg_on{};
		bool is_vip{};

		util::Time use_reset_time{};

		game::Array<int, 10> weapon_ammo{};
		int weapon_clip{};
		game::weapon::ID current_weapon{};

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
		{
		}

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

		game::StatusIcon DisplayingStatusIcon() const POKEBOT_NOEXCEPT { return status_icon; }
		weapon::ID CurrentWeapon() const POKEBOT_NOEXCEPT { return current_weapon; }
		int CurrentWeaponClip() const POKEBOT_NOEXCEPT { return weapon_clip; }

		edict_t* Edict() POKEBOT_NOEXCEPT { return client; }
		const edict_t* Edict() const POKEBOT_NOEXCEPT { return client; }
		operator edict_t* () POKEBOT_NOEXCEPT { return Edict(); }
		operator const edict_t* () const POKEBOT_NOEXCEPT { return client; }

		const char* const ClassName() const POKEBOT_NOEXCEPT { return STRING(client->v.classname); }
		const char* const Name() const POKEBOT_NOEXCEPT { return STRING(client->v.netname); }

		void PressKey(const int Key) noexcept {
			if (bool(Key & IN_USE)) {
				use_reset_time = gpGlobals->time + 1.0f;
			}
			client->v.button |= Key;
		}

		void ResetKey() noexcept {
			if (use_reset_time > gpGlobals->time) {
				client->v.button &= IN_USE;
			} else {
				client->v.button = 0;
			}
		}


		bool IsValid() const POKEBOT_NOEXCEPT { return IsValid(client); }
		bool IsDead() const POKEBOT_NOEXCEPT { return IsDead(client); }

		bool IsVIP() const POKEBOT_NOEXCEPT { return is_vip; }
		int WeaponAmmo(const weapon::ammo::ID Ammo_ID) const POKEBOT_NOEXCEPT { return weapon_ammo[static_cast<int>(Ammo_ID) - 1]; }

		bool IsInBuyzone() const noexcept { return bool(DisplayingStatusIcon() & StatusIcon::Buy_Zone); }
		bool IsInEscapezone() const noexcept { return bool(DisplayingStatusIcon() & StatusIcon::Escape_Zone); }
		bool IsInRescuezone() const noexcept { return bool(DisplayingStatusIcon() & StatusIcon::Rescue_Zone); }
		bool IsInVipSafety() const noexcept { return bool(DisplayingStatusIcon() & StatusIcon::Vip_Safety); }
		bool HasDefuser() const noexcept { return bool(DisplayingStatusIcon() & StatusIcon::Defuser); }

		bool IsWalking() const noexcept { return bool(velocity.Length2D() <= 150.0f); }
		bool IsStopping() const noexcept { return bool(velocity.Length2D() <= 10.0f); }
		bool IsDucking() const POKEBOT_NOEXCEPT { return bool(button & IN_DUCK); }
		bool IsInWater() const POKEBOT_NOEXCEPT { return bool(flags & FL_INWATER); }
		bool IsOnFloor() const POKEBOT_NOEXCEPT { return bool(flags & (FL_ONGROUND | FL_PARTIALGROUND)) != 0; }
		bool IsOnTrain() const POKEBOT_NOEXCEPT { return bool(flags & FL_ONTRAIN); }
		bool IsFiring() const POKEBOT_NOEXCEPT { return bool(button & IN_ATTACK); }
		bool IsReadyToThrowGrenade() const POKEBOT_NOEXCEPT { return IsFiring() && bool(weapons & weapon::Grenade_Bit); }
		bool IsPlantingBomb() const POKEBOT_NOEXCEPT { return IsFiring() && bool(weapons & weapon::C4_Bit) && (sequence == 63 || sequence == 61); }
		bool IsClimblingLadder() const POKEBOT_NOEXCEPT { return (movetype & MOVETYPE_FLY); }

		bool HasWeapon(const weapon::ID Weapon_ID) const noexcept { return bool(weapons & game::ToBit<int>(Weapon_ID)); }
		bool HasPrimaryWeapon() const noexcept { return weapons & weapon::Primary_Weapon_Bit; }
		bool HasSecondaryWeapon() const noexcept { return weapons & weapon::Secondary_Weapon_Bit; }

		/**
		* @brief Check the clip of current weapon remains or not
		* @return
		*/
		bool IsOutOfClip() const POKEBOT_NOEXCEPT { return (weapon::Weapon_Type[static_cast<int>(CurrentWeapon())] == weapon::WeaponType::Primary || weapon::Weapon_Type[static_cast<int>(CurrentWeapon())] == weapon::WeaponType::Secondary) && CurrentWeaponClip() <= 0; }
		bool IsOutOfAmmo(const weapon::ammo::ID Ammo_ID) const POKEBOT_NOEXCEPT { return WeaponAmmo(Ammo_ID) <= 0; }
		bool IsOutOfCurrentWeaponAmmo() const POKEBOT_NOEXCEPT {
			return
				(weapon::Weapon_Type[static_cast<int>(CurrentWeapon())] == weapon::WeaponType::Primary ||
				 weapon::Weapon_Type[static_cast<int>(CurrentWeapon())] == weapon::WeaponType::Secondary) &&
				IsOutOfAmmo(std::get<weapon::ammo::ID>(weapon::Weapon_CVT[static_cast<int>(CurrentWeapon()) - 1]));
		}



		bool CanSeeEntity(const edict_t* const Target) const noexcept {
			return entity::CanSeeEntity(Edict(), Target);
		}

		game::Team GetTeamFromModel() const POKEBOT_NOEXCEPT {
			return game::GetTeamFromModel(client);
		}

		bool IsPlayerModelReloading() const POKEBOT_NOEXCEPT {
			// The value of entvars_t::sequence.
			// sequence has two values. The value changes depending on whether the player is standing or not.
			// 
			// sequence[weapon_id][0] is the value when the player is standing.
			// sequence[weapon_id][1] is the value when the player is ducking.
			static int sequence[][2]{
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

		bool IsViewModelReloading() const POKEBOT_NOEXCEPT {
			// The value of entvars_t::weapon_anim.
			// weaponanim has two values, weapons with different animation values ​​are as follows:
			//	1. Glock(This is the only weapon the reload animation changes randomly).
			//  2. USP with or without silencer
			//	3. M4A1 with or without silencer
			//
			// weaponanim[weapon_id][0] is the value without silencer.
			// weaponanim[weapon_id][1] is the value with silencer.
			static int weaponanim[][2]{
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

		game::Team GetTeam() const noexcept { return game::GetTeamFromModel(client); }
		bool IsFakeClient() const noexcept { return bool(flags & util::Third_Party_Bot_Flag); }

		void on_money_changed(const int Money) POKEBOT_NOEXCEPT {
			money = Money;
		}

		
		void on_nvg_changed(const bool nvg) POKEBOT_NOEXCEPT {
			is_nvg_on = nvg;
		}
	};
}