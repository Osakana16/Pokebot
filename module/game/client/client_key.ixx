module;
#define _WIN32 1
#include <extdll.h>
#include <dllapi.h>
#include <meta_api.h>
#include <entity_state.h>

export module pokebot.game.client:client_key;

export namespace pokebot::game::client {
	class ClientKey final {
		const int Client_Index{};
		char* const infobuffer{};
	public:
		ClientKey(edict_t* target) POKEBOT_NOEXCEPT :
			Client_Index(ENTINDEX(target)),
			infobuffer((*g_engfuncs.pfnGetInfoKeyBuffer)(target)) {
		}

		ClientKey& SetValue(const char* Key, const char* Value) POKEBOT_NOEXCEPT {
			(*g_engfuncs.pfnSetClientKeyValue)(Client_Index, infobuffer, Key, Value);
			return *this;
		}
	};
}