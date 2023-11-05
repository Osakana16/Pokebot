#include "config.hpp"
namespace pokebot::config {
	cvar_t* CreateCVar(const char* const Name) {
		static std::list<cvar_t> registrations{};
		auto cvar = g_engfuncs.pfnCVarGetPointer(Name);
		if (cvar == nullptr) {
			registrations.push_back({});
			registrations.back().name = Name;
			registrations.back().flags = FCVAR_SERVER;
			g_engfuncs.pfnCVarRegister(&registrations.back());
			cvar = g_engfuncs.pfnCVarGetPointer(Name);
		}
		return cvar;
	}
}