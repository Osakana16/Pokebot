export module pokebot.plugin.console.command: console_command_register;
import :pk_bot;
import :pk_menu;

#define REGISTER_COMMAND(NAME) REG_SVR_COMMAND(#NAME, NAME);

export namespace pokebot::plugin::console {
	class CommandRegister {
	public:
		CommandRegister() {
			REGISTER_COMMAND(pk_add);
			REGISTER_COMMAND(pk_add_ct);
			REGISTER_COMMAND(pk_add_t);
			REGISTER_COMMAND(pk_menu);

			REGISTER_COMMAND(pk_kill);
			REGISTER_COMMAND(pk_kill_t);
			REGISTER_COMMAND(pk_kill_ct);
		}
	};
}