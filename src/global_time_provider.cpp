#include "util/global_time_provider.hpp"

namespace pokebot::util {
	util::Time GetRealGlobalTime() noexcept {
		return gpGlobals->time;
	}
}