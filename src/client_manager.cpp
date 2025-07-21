module pokebot.game.client.manager;
import pokebot.game.client;

import pokebot.game.weapon;
import pokebot.game.util;
import pokebot.util;
import pokebot.game.player;
import pokebot.game.cs.team;
import pokebot.game.cs.model;

namespace pokebot::game::client {
	ClientManager::ClientManager(plugin::Observables* plugin_observables, engine::Observables* engine_observables) {
		plugin_observables->frame_update_observable.AddObserver(std::make_shared<common::NormalObserver<void>>([&] {
			for (auto& client : clients) {
				client.second.button = 0;
			}
		}));

		plugin_observables->client_connection_observable.AddObserver(std::make_shared<common::NormalObserver<plugin::event::ClientInformation>>([&](const plugin::event::ClientInformation&) {

		}));

		plugin_observables->client_disconnection_observable.AddObserver(std::make_shared<common::NormalObserver<plugin::event::ClientInformation>>([&](const plugin::event::ClientInformation&) {

		}));

		plugin_observables->on_add_bot_command_fired_observable.AddObserver(std::make_shared<common::NormalObserver<std::tuple<util::PlayerName, game::Team, game::Model>>>([&](const std::tuple<util::PlayerName, game::Team, game::Model>& sender) {
			Create(std::get<util::PlayerName>(sender).c_str());
		}));

		engine_observables->new_round_observable.AddObserver(std::make_shared<common::NormalObserver<void>>([&] {
			for (auto& client : clients) {
				client.second.is_nvg_on = false;
			}
		}));
#if 0		
		engine_observables->item_get.AddObserver(std::make_shared<common::NormalObserver<std::tuple<const edict_t* const, game::StatusIcon>>>([&](const std::tuple<const edict_t* const, game::StatusIcon>& args) {
			auto player_name = STRING(std::get<0>(args)->v.netname);
			if (auto entity = Get(player_name); entity != nullptr) {
				entity->item  |= std::get<1>(args);
			}
		}));
#endif
		engine_observables->status_icon_observable.AddObserver(std::make_shared<common::NormalObserver<std::tuple<const edict_t* const, game::StatusIcon>>>([&](const std::tuple<const edict_t* const, game::StatusIcon>& args) {
			auto player_name = STRING(std::get<0>(args)->v.netname);
			if (auto entity = Get(player_name); entity != nullptr) {
				entity->status_icon |= std::get<1>(args);
			}
		}));
	}

	auto& ClientManager::GetAll() const {
		return clients;
	}

	const Client* const ClientManager::Get(const std::string_view& Name) const {
		if (auto it = clients.find(Name.data()); it != clients.end()) {
			return &it->second;
		}
		return nullptr;
	}

	Client* const ClientManager::Get(const std::string_view& Name) {
		if (auto it = clients.find(Name.data()); it != clients.end()) {
			return &it->second;
		}
		return nullptr;
	}

	bool ClientManager::Disconnect(const char* const Name) noexcept {
		return clients.erase(Name) > 0;
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

	void ClientManager::OnDeath(const std::string_view Client_Name) {
		decltype(auto) target = Get(Client_Name.data());
		target->status_icon = StatusIcon::Not_Displayed;
		target->item = Item::None;
	}

	void ClientManager::OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) {
		if (auto target = Get(Client_Name.data()); target != nullptr) {
			if (target->health - Health <= 0) {
				OnDeath(Client_Name);
			} else {
				// TODO: Send the event message for a bot.
			}
		}
	}

	void ClientManager::OnMoneyChanged(const std::string_view Client_Name, const int Money) {
		Get(Client_Name.data())->money = Money;
	}

	void ClientManager::OnScreenFaded(const std::string_view Client_Name) {

	}

	void ClientManager::OnNVGToggled(const std::string_view Client_Name, const bool Toggle) {
		Get(Client_Name.data())->is_nvg_on = Toggle;
	}

	void ClientManager::OnWeaponChanged(const std::string_view Client_Name, const game::weapon::ID Weapon_ID) {
		Get(Client_Name.data())->current_weapon = Weapon_ID;
	}

	void ClientManager::OnClipChanged(const std::string_view Client_Name, const game::weapon::ID Weapon_ID, const int Amount) {
		Get(Client_Name.data())->weapon_clip = Amount;
	}

	void ClientManager::OnAmmoPickedup(const std::string_view Client_Name, const game::weapon::ammo::ID Ammo_ID, const int Amount) {
		Get(Client_Name.data())->weapon_ammo[static_cast<int>(Ammo_ID)] = Amount;
	}

	void ClientManager::OnTeamAssigned(const std::string_view Client_Name, const game::Team Assigned_Team) {
		auto target = Get(Client_Name.data());
		if (target != nullptr)
			target->team = Assigned_Team;
	}

	void ClientManager::OnVIPChanged(const std::string_view Client_Name) {
		auto&& candidate = Get(Client_Name.data());
		candidate->is_vip = true;
	}

	void ClientManager::OnDefuseKitEquiped(const std::string_view Client_Name) {
		Get(Client_Name.data())->item |= Item::Defuse_Kit;
	}
}