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

	CalculatableHLVector ToCalculatable(HLVector2D vector) {
		return CalculatableHLVector{ vector.x, vector.y };
	}

	CalculatableHLVector ToCalculatable(HLVector vector) {
		return CalculatableHLVector{ vector.x, vector.y, vector.z };
	}

	HLVector2D ToHLVector2D(const CalculatableHLVector& vector) {
		return HLVector2D{ vector[0], vector[1] };
	}

	HLVector ToHLVector(const CalculatableHLVector& vector) {
		return HLVector{ vector[0], vector[1], vector[2] };
	}
}