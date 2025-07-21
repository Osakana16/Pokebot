export module pokebot.game.scenario.manager;

export namespace pokebot::game::scenario {
	class ScenarioManager {
	public:
		virtual void Update() = 0;
	};
}