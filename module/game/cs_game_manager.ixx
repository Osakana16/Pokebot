export module pokebot.game: cs_game_manager;
import :game_manager_base;

import pokebot.game.util;

export namespace pokebot::game {
	/**
	* @brief Game class for Counter-Strike
	*/
	class CSGameBase : public GameBase {
	public:
		inline virtual ~CSGameBase() override = 0 {}

		// - Hostage -

		/**
		* @brief Get the number of hostages.
		* @return Number of hostages.
		*/
		virtual size_t GetNumberOfHostages() const = 0;

		/**
		* @brief Get the number of hostages who are not dead.
		* @return Number of living hostages.
		*/
		virtual size_t GetNumberOfLivingHostages() const = 0;

		/**
		* @brief Get the number of hostages who are not dead.
		* @return Number of living hostages.
		*/
		virtual size_t GetNumberOfRescuedHostages() const = 0;

		/**
		* @brief Check the hostage is used or not.
		* @param Index The hostage index
		* @return True if the hostage is leading by a player, false if it's not.
		*/
		virtual bool IsHostageUsed(const int Index) const = 0;

		/**
		* @brief Check the hostage is used by the specified player.
		* @param Index The hostage index
		* @param Owner_Name The player name
		* @return True if the hostage owned by the specified player, false if it's not.
		*/
		virtual bool IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name) = 0;

		virtual pokebot::game::MapFlags GetScenario() const = 0;
	};
}