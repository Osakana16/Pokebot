export module pokebot.game.scenario: demolition_manager;
import pokebot.game.scenario.manager;

import pokebot.util;

export namespace pokebot::game::scenario {
	class DemolitionManager : public ScenarioManager {
		std::optional<Vector> c4_origin;
		std::optional<Vector> backpack_origin;
		util::PlayerName bomber_name;
	public:
		void Update() final;
		
		std::optional<Vector> GetC4Origin() const noexcept { return c4_origin; }
		std::optional<Vector> GetBackpackOrigin() const noexcept { return backpack_origin; }
	};
}