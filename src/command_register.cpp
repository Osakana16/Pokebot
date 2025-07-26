module;
#include "goldsrc.hpp"
module pokebot.plugin.console.command: console_command_register;

#define REGISTER_COMMAND(NAME) REG_SVR_COMMAND(#NAME, NAME);
namespace pokebot::plugin::console {
	CommandRegister::CommandRegister() {
		REGISTER_COMMAND(pk_add);
		REGISTER_COMMAND(pk_add_ct);
		REGISTER_COMMAND(pk_add_t);
		REGISTER_COMMAND(pk_menu);

		REGISTER_COMMAND(pk_kill);
		REGISTER_COMMAND(pk_kill_t);
		REGISTER_COMMAND(pk_kill_ct);
	}
}