module;
#include "goldsrc.hpp"
export module pokebot.plugin.event: client_information;
import pokebot.common.event_handler;


export namespace pokebot::plugin::event {
	struct ClientInformation final {
		const edict_t* const entity;
		const char* Address;
	};

	using ClientInformationObservable = common::Observable<ClientInformation>;
}