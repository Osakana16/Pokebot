module;
export module pokebot.game.scenario: demolition_manager;
import pokebot.game.scenario.manager;
import pokebot.engine.hlvector;
import pokebot.util;

export namespace pokebot::game::scenario {
	class DemolitionManager : public ScenarioManager {
		std::optional<engine::HLVector> c4_origin;
		std::optional<engine::HLVector> backpack_origin;
		util::PlayerName bomber_name;
	public:
		void Update() final;
		
		std::optional<engine::HLVector> GetC4Origin() const noexcept { return c4_origin; }
		std::optional<engine::HLVector> GetBackpackOrigin() const noexcept { return backpack_origin; }
	};
}