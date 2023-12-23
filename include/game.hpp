#pragma once
#include "database.hpp"
namespace pokebot {
	namespace game {
		inline bool is_enabled_auto_waypoint = true;

		enum class StatusIcon {
			Not_Displayed,
			Buy_Zone = 1 << 0,
			Defuser = 1 << 1,
			C4 = 1 << 2,
			Rescue_Zone = 1 << 3,
			Vip_Safety = 1 << 4,
			Escape_Zone = 1 << 5
		};
		POKEBOT_ENUM_BIT_OPERATORS(StatusIcon);

		enum class Item {
			None,
			Nightvision=1<<0,
			Defuse_Kit=1<<1
		};
		POKEBOT_ENUM_BIT_OPERATORS(Item);

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

		constexpr const char* const Weapon_CVT[30]{
			"weapon_p228",
			"weapon_shield",
			"weapon_scout",
			"weapon_hegrenade",
			"weapon_xm1014",
			"weapon_c4",
			"weapon_aug",
			"weapon_mac10",
			"weapon_smoke",
			"weapon_elite",
			"weapon_fiveseven",
			"weapon_ump45",
			"weapon_sg550",
			"weapon_galil",
			"weapon_famas",
			"weapon_usp",
			"weapon_glock18",
			"weapon_awp",
			"weapon_mp5navy",
			"weapon_m249",
			"weapon_m3",
			"weapon_m4a1",
			"weapon_tmp",
			"weapon_g3sg1",
			"weapon_flashbang",
			"weapon_deagle",
			"weapon_sg552",
			"weapon_ak47",
			"weapon_knife",
			"weapon_p90"
		};

		class Client final {
			friend class ClientManager;

			edict_t* client{};
			bool is_bot{};
			int& button;

			common::Team team{};
			int money{};
			StatusIcon status_icon{};
			Item item{};
			bool is_nvg_on{};

			struct {
				int clip{};
				int ammo{};
			} weapon[32]{};
			Weapon current_weapon{};
		public:
			static bool IsDead(const edict_t* const Target) noexcept { return Target->v.deadflag == DEAD_DEAD || Target->v.health <= 0 || Target->v.movetype == MOVETYPE_NOCLIP; }
			static bool IsValid(const edict_t* const Target) noexcept { return (Target != nullptr && !Target->free); }
			static int Index(const edict_t* const Target) noexcept { return ENTINDEX(const_cast<edict_t*>(Target)); }
		public:
			Client(edict_t* e, bool is_bot_) noexcept :
				client(e),
				origin(client->v.origin),
				angles(client->v.angles),
				avelocity(client->v.avelocity),
				punchangle(client->v.punchangle),
				v_angle(client->v.v_angle),
				ideal_yaw(client->v.ideal_yaw),
				idealpitch(client->v.idealpitch),
				button(client->v.button),
				view_ofs(client->v.view_ofs),
				Health(client->v.health),
				Max_Health(client->v.max_health),
				Speed(client->v.speed) {
				is_bot = is_bot_;
			}

			static std::shared_ptr<Client> Create(std::string client_name);
			static std::shared_ptr<Client> Attach(edict_t*, const bool Is_Bot),
				Attach(const int, const bool Is_Bot);

			edict_t* Edict() noexcept { return client; }
			operator edict_t* () noexcept { return Edict(); }
			operator const edict_t* () const noexcept { return client; }

			bool IsBot() const noexcept { return is_bot; }
			bool IsValid() const noexcept { return IsValid(client); }
			bool IsDead() const noexcept { return IsDead(client); }

			bool IsDucking() const noexcept { return (client->v.flags & FL_DUCKING); }
			bool IsInWater() const noexcept { return (client->v.flags & FL_INWATER); }
			bool IsOnFloor() const noexcept { return (client->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)) != 0; }
			bool IsOnTrain() const noexcept { return (client->v.flags & FL_ONTRAIN); }

			int Index() const noexcept { return Index(client); }

			auto Button() const noexcept { return client->v.button; }
			auto Impulse() const noexcept { return client->v.impulse; }
			const char* ClassName() const noexcept { return STRING(client->v.classname); }
			std::string_view Name() const noexcept { return STRING(client->v.netname); }
			void PressKey(const int Key) noexcept { client->v.button |= Key; }
			common::Team GetTeam() const noexcept { return common::GetTeamFromModel(client); }
			bool IsShowingIcon(const StatusIcon icon) const noexcept { return bool(status_icon & icon); }

			const float& Health;
			const float& Max_Health;
			const float& Speed;
			const int& Money = money;

			Vector& view_ofs;
			Vector& origin;
			Vector& angles;
			Vector& avelocity;
			Vector& punchangle;
			Vector& v_angle;
			float& ideal_yaw;
			float& idealpitch;
		};

		// The status in the game
		class ClientStatus {
			std::shared_ptr<Client> client{};
		public:
			ClientStatus(std::shared_ptr<Client>&);

			common::Team GetTeam() const noexcept;
			bool CanSeeFriend() const noexcept;
			bool CanSeeEnemy() const noexcept;
			std::vector<const edict_t*> GetEntitiesInView() const noexcept;
		};

		class ClientManager {
			std::unordered_map<std::string, std::shared_ptr<Client>> clients{};
		public:
			ClientStatus GetClientStatus(std::string_view client_name);
			std::shared_ptr<Client> Create(std::string client_name);
			std::shared_ptr<Client> Register(edict_t*, bool is_bot);
			auto& GetAll() {
				return clients;
			}

			std::shared_ptr<Client> Get(const std::string& Name) noexcept {
				if (auto it = clients.find(Name); it != clients.end())
					return it->second;

				return nullptr;
			}

			void OnDeath(const std::string_view Client_Name) noexcept;
			void OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) noexcept;
			void OnMoneyChanged(const std::string_view Client_Name, const int) noexcept;
			void OnScreenFaded(const std::string_view Client_Name) noexcept;
			void OnNVGToggled(const std::string_view Client_Name, const bool) noexcept;
			void OnWeaponChanged(const std::string_view Client_Name, const game::Weapon) noexcept;
			void OnClipChanged(const std::string_view Client_Name, const game::Weapon, const int) noexcept;
			void OnAmmoPickedup(const std::string_view Client_Name, const game::Weapon, const int) noexcept;
			void OnTeamAssigned(const std::string_view Client_Name, common::Team) noexcept;
			void OnItemChanged(const std::string_view Client_Name, game::Item) noexcept;
			void OnStatusIconShown(const std::string_view Client_Name, const StatusIcon) noexcept;
		};

		struct Sound final {
			Vector origin{};
			int volume{};
		};

		enum class MapFlags {
			Assassination = 1 << 0,
			HostageRescue = 1 << 1,
			Demolition = 1 << 2,
			Escape = 1 << 3
		};
		POKEBOT_ENUM_BIT_OPERATORS(MapFlags);

		class Hostage final {
			Hostage() = default;
			Hostage(const Hostage&);
			Hostage& operator=(const Hostage&) = delete;

			const edict_t* entity;
			std::shared_ptr<Client> owner;
		public:
			operator const edict_t* const () const noexcept {
				return entity;
			}
			void Update() noexcept;
			bool RecoginzeOwner(std::shared_ptr<Client>&) noexcept;

			bool IsUsed() const noexcept { return owner != nullptr; }
			const bool IsOwnedBy(const std::string_view& Name) const noexcept { return (owner != nullptr && owner->Name() == Name); }
	 		bool IsReleased() const noexcept { return (entity->v.effects & EF_NODRAW); }
			static Hostage AttachHostage(const edict_t*) noexcept;
			const Vector& Origin() const noexcept {
				return entity->v.origin;
			}

			Hostage(Hostage&& h) noexcept {
				owner = std::move(h.owner);
				entity = h.entity;
				h.entity = nullptr;
			}
		};

		inline class Game {
			database::Database database{};
			std::vector<Hostage> hostages{};

			std::vector<std::string> bot_args{};
			const edict_t* Host_Client{};
			MapFlags map_flags{};
			uint32_t round{};
			bool is_newround{};
		public:
			ClientManager clients{};

			size_t GetHostageNumber() const noexcept {
				return hostages.size();
			}

			bool IsHostageUsed(const int Index) const noexcept {
				return hostages[Index].IsUsed();
			}

			bool IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name) {
				return hostages[Index].IsOwnedBy(Owner_Name);
			}

			const edict_t* const GetClosedHostage(const Vector& Origin, const float Base_Distance) {
				for (auto& hostage : hostages) {
					if (common::Distance(hostage.Origin(), Origin) <= Base_Distance) {
						return hostage;
					}
				}

			}

			inline const std::string& GetBotArg(const size_t Index) const noexcept {
				return bot_args[Index];
			}

			inline size_t GetBotArgCount() const noexcept {
				return bot_args.size();
			}

			inline bool IsBotCmd() const noexcept {
				return !bot_args.empty();
			}

			inline void SetHost(const edict_t* Host) noexcept {
				Host_Client = Host;
			}

			const edict_t* GetHost() const noexcept {
				return Host_Client;
			}

			bool HasHost() const noexcept {
				return GetHost() != nullptr;
			}

			auto CurrentRonud() const noexcept {
				return round;
			}

			bool IsCurrentMode(const MapFlags) const noexcept;
			void IssueCommand(edict_t* client, const std::string& Sentence) noexcept;

			void Init(edict_t* entities, int max);
			void Update();

			// - Event funcs -

			void OnNewRound() noexcept;
		} game{};
	}

	namespace entity {
		bool CanSeeEntity(edict_t* const self, const const edict_t* Target) noexcept;
		bool InViewCone(edict_t* const self, const Vector& Origin) noexcept;
		bool IsVisible(edict_t* const Self, const Vector& Origin) noexcept;
	}

	namespace engine {
		class ClientController {
		public:
			bool Connect();

		};

		class ClientKey final {
			const int Client_Index{};
			char* const infobuffer{};
		public:
			ClientKey(edict_t* target) noexcept;
			ClientKey& SetValue(const char* Key, const char* Value) noexcept;
		};
	}
}