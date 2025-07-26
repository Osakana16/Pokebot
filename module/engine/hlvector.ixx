export module pokebot.engine.hlvector;
export namespace pokebot::engine {
	struct HLVector2D {
		float x, y;
	};

	struct HLVector {
		float x, y, z;
	};

	HLVector operator+(const HLVector& lhs, const HLVector& rhs) noexcept;
	HLVector operator-(const HLVector& lhs, const HLVector& rhs) noexcept;
	HLVector operator*(const HLVector& lhs, const float f) noexcept;
}