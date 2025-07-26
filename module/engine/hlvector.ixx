export module pokebot.engine.hlvector;
import std;

export namespace pokebot::engine {
	struct HLVector2D {
		float x, y;
	};

	struct HLVector {
		float x, y, z;
	};

	using CalculatableHLVector = std::valarray<float>;
	CalculatableHLVector ToCalculatable(HLVector2D vector);
	CalculatableHLVector ToCalculatable(HLVector vector);
	HLVector2D ToHLVector2D(const CalculatableHLVector& vector);
	HLVector ToHLVector(const CalculatableHLVector& vector);
}