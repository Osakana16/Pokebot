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

		virtual pokebot::game::MapFlags GetScenario() const = 0;
	};
}