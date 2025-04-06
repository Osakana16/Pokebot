#pragma once

namespace pokebot::util {
	common::Time GetRealGlobalTime() noexcept;
	inline common::Time GetMockGlobalTime() noexcept { return 0.0f; }
}