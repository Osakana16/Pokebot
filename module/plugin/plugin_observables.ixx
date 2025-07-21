module;
#include <tuple>

export module pokebot.plugin.observables;
import pokebot.common.event_handler;
import pokebot.plugin.event;
import pokebot.plugin.event.server_activation;
import pokebot.game.util;
import pokebot.util;
import pokebot.game.cs.team;
import pokebot.game.cs.model;

export namespace pokebot::plugin {
	struct Observables {
		common::NormalObservable<plugin::event::ClientInformation> client_connection_observable;
		common::NormalObservable<plugin::event::ClientInformation> client_disconnection_observable;

		common::NormalObservable<edict_t*> client_put_in_server_observable;
		common::NormalObservable<util::fixed_string<256u>> map_loaded_observable;
		common::NormalObservable<plugin::event::EdictList> server_activation_observable;
		common::NormalObservable<void> frame_update_observable;
		common::NormalObservable<void> game_init_observable;
		common::NormalObservable<edict_t*> entity_spawn_obserable;

		common::NormalObservable<std::tuple<util::PlayerName, game::Team, game::Model>> on_add_bot_command_fired_observable;
	};
}