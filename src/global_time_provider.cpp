#include "util/global_time_provider.hpp"

namespace pokebot::util {
	common::Time GetRealGlobalTime() noexcept {
		return gpGlobals->time;
	}
}