#pragma once

namespace pokebot::util {
	float GetRealGlobalTime() noexcept;
	inline float GetMockGlobalTime() noexcept { return 0.0f; }
}