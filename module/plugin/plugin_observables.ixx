export module pokebot.plugin.observables;
import pokebot.common.event_handler;
import pokebot.plugin.event;
import pokebot.plugin.event.server_activation;
import pokebot.util;

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
	};
}