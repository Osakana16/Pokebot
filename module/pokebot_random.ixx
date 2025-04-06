export module pokebot.util.random;
#include <random>

namespace pokebot::util {
	export template<typename T>
	class Random {};

	export template<>
	class Random<int> {
		std::random_device device{};
		std::mt19937 mt;
		std::uniform_int_distribution<int> distribution;
	public:
		Random(const int min, const int max) : mt(device()), distribution(min, max) {}

		template<typename T>
		operator T() { return distribution(mt); }
	};

	export template<>
	class Random<float> {
		std::random_device device{};
		std::mt19937 mt;
		std::uniform_real_distribution<float> distribution;
	public:
		Random(const float min, const float max) : mt(device()), distribution(min, max) {}

		template<typename T>
		operator T() { return distribution(mt); }
	};
}