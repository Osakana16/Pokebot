export module pokebot.bot.state: state_element;

export namespace pokebot::bot::state {
	class StateElement {
	public:
		virtual ~StateElement() = 0 {}

		virtual void Run() = 0;
	};
}
