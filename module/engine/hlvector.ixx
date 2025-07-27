module;
#include "extdll.h"
#include <cstddef>
export module pokebot.engine.hlvector;
export namespace pokebot::engine {
	struct HLVector2D {
		float x;
		float y;
	};

	struct HLVector {
		float x;
		float y;
		float z;
	};

	HLVector operator+(const HLVector& lhs, const HLVector& rhs) noexcept;
	HLVector operator-(const HLVector& lhs, const HLVector& rhs) noexcept;
	HLVector operator*(const HLVector& lhs, const float f) noexcept;

	template <class T>
	concept VectorConvertible = requires(T t) {
		{ t.x } -> std::same_as<float&>;
		{ t.y } -> std::same_as<float&>;
		{ t.z } -> std::same_as<float&>;
	} && sizeof(T) >= sizeof(::Vector);
}