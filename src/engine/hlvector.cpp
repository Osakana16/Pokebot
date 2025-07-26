module;
#include "extdll.h"
module pokebot.engine.hlvector;

namespace pokebot::engine {
	// HLVector shoould be same size.
	static_assert(sizeof(HLVector2D) == sizeof(::Vector2D));
	// HLVector should be same data alignment.
	static_assert(alignof(HLVector2D) == alignof(::Vector2D));

	// HLVector shoould be same size.
	static_assert(sizeof(HLVector) == sizeof(::Vector));
	// HLVector should be same data alignment.
	static_assert(alignof(HLVector) == alignof(::Vector));


	HLVector operator+(const HLVector& lhs, const HLVector& rhs) noexcept {
		return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
	}

	HLVector operator-(const HLVector& lhs, const HLVector& rhs) noexcept {
		return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
	}

	HLVector operator*(const HLVector& lhs, const float f) noexcept {
		return { lhs.x * f, lhs.y * f, lhs.z * f };
	}
}