module;
#include <tuple>

export module pokebot.game.client: client_manager;
import :client;

import pokebot.game.player;
import pokebot.game.weapon;
import pokebot.game.util;
import pokebot.util;

export namespace pokebot::game::client {
	using ClientCreationResult = std::tuple<bool, pokebot::util::PlayerName>;
	class ClientManager {
		std::unordered_map<pokebot::util::PlayerName, Client, pokebot::util::PlayerName::Hash> clients{};
	public:
		auto& GetAll() const;

		const Client* Get(const char* const Name) const;
		Client* GetAsMutable(const char* const Name);
		bool Disconnect(const char* const Name) noexcept;
		void OnNewRound();

		ClientCreationResult Create(std::string_view client_name);
		bool Register(edict_t* edict);
		void OnDeath(const std::string_view Client_Name);
		void OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit);
		void OnMoneyChanged(const std::string_view Client_Name, const int Money);
		void OnScreenFaded(const std::string_view Client_Name);
		void OnNVGToggled(const std::string_view Client_Name, const bool Toggle);
		void OnWeaponChanged(const std::string_view Client_Name, const game::weapon::ID Weapon_ID);
		void OnClipChanged(const std::string_view Client_Name, const game::weapon::ID Weapon_ID, const int Amount);
		void OnAmmoPickedup(const std::string_view Client_Name, const game::weapon::ammo::ID Ammo_ID, const int Amount);
		void OnTeamAssigned(const std::string_view Client_Name, const game::Team Assigned_Team);
		void OnItemChanged(const std::string_view Client_Name, game::Item item);
		void OnStatusIconShown(const std::string_view Client_Name, const game::StatusIcon Icon);
		void OnVIPChanged(const std::string_view Client_Name);
		void OnDefuseKitEquiped(const std::string_view Client_Name);
	};
}