import pokebot.game.util;
import pokebot.util;

export module pokebot.game.client: client_manager;
import :client;
import :client_key;

export namespace pokebot::game::client {
	class ClientManager {
		std::unordered_map<pokebot::util::PlayerName, Client, pokebot::util::PlayerName::Hash> clients{};
	public:
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


		void OnNewRound() {
			for (auto& client : clients) {
				client.second.is_nvg_on = false;
			}
		}

		ClientCreationResult Create(std::string_view client_name) {
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

		bool Register(edict_t* edict) {
			if (auto it = clients.find(STRING(edict->v.netname)); it == clients.end()) {
				clients.emplace(STRING(edict->v.netname), edict);
				return true;
			}
			return false;
		}

		void OnDeath(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			decltype(auto) target = GetAsMutable(Client_Name.data());
			target->status_icon = StatusIcon::Not_Displayed;
			target->item = Item::None;
		}

		void OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) POKEBOT_NOEXCEPT {
			if (auto target = GetAsMutable(Client_Name.data()); target != nullptr) {
				if (target->health - Health <= 0) {
					OnDeath(Client_Name);
				} else {
					// TODO: Send the event message for a bot.
				}
			}
		}

		void OnMoneyChanged(const std::string_view Client_Name, const int Money) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->money = Money;
		}

		void OnScreenFaded(const std::string_view Client_Name) POKEBOT_NOEXCEPT {

		}

		void OnNVGToggled(const std::string_view Client_Name, const bool Toggle) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->is_nvg_on = Toggle;
		}

		void OnWeaponChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->current_weapon = Weapon_ID;
		}

		void OnClipChanged(const std::string_view Client_Name, const game::Weapon Weapon_ID, const int Amount) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->weapon_clip = Amount;
		}

		void OnAmmoPickedup(const std::string_view Client_Name, const game::AmmoID Ammo_ID, const int Amount) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->weapon_ammo[static_cast<int>(Ammo_ID)] = Amount;
		}

		void OnTeamAssigned(const std::string_view Client_Name, const game::Team Assigned_Team) POKEBOT_NOEXCEPT {
			auto target = GetAsMutable(Client_Name.data());
			if (target != nullptr)
				target->team = Assigned_Team;
		}

		void OnItemChanged(const std::string_view Client_Name, game::Item item) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->item |= item;
		}

		void OnStatusIconShown(const std::string_view Client_Name, const StatusIcon Icon) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->status_icon |= Icon;
		}

		void OnVIPChanged(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			auto&& candidate = GetAsMutable(Client_Name.data());
			candidate->is_vip = true;
		}

		void OnDefuseKitEquiped(const std::string_view Client_Name) POKEBOT_NOEXCEPT {
			GetAsMutable(Client_Name.data())->item |= Item::Defuse_Kit;
		}
	};
}