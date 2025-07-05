export module pokebot.plugin.event.server_activation;
import pokebot.plugin.event;

import pokebot.common.event_handler;

export namespace pokebot::plugin::event {
	struct EdictList {
		edict_t* edict_list;
		int edict_count;
		int client_max;
	};
}