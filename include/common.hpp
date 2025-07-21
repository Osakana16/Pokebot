#pragma once
#include <sstream>

#define POKEBOT_ENUM_BIT_OPERATORS(TYPE) inline constexpr TYPE operator|(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 | (int)n2); } inline constexpr TYPE operator&(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 & (int)n2); } inline constexpr TYPE operator^(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 ^ (int)n2); } inline constexpr TYPE operator|=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 | n2); return n1; } inline constexpr TYPE operator&=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 & n2); return n1; } inline constexpr TYPE operator^=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 ^ n2); return n1; } inline constexpr TYPE operator~(TYPE n1) noexcept { return static_cast<TYPE>(~static_cast<int>(n1)); }
#define POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(TYPE,...) enum class TYPE { __VA_ARGS__ }; POKEBOT_ENUM_BIT_OPERATORS(TYPE)

#ifndef NDEBUG
#define POKEBOT_NOEXCEPT noexcept(false)
#else
#define POKEBOT_NOEXCEPT noexcept(true)
#endif
#define DEBUG_PRINTF(...) SERVER_PRINT(std::format(__VA_ARGS__).c_str())

#define USE_STDARRAY false

namespace pokebot{
	namespace game {
#ifndef USE_STDARRAY
		template<typename T, size_t N>
		using Array = std::array<T, N>;
#else
		template<typename T, size_t N>
		using Array = T[N];
#endif

		using Dec = float;

		class VecElem {
			float value{};
		public:
			VecElem(float v) : value(v) {}

			VecElem& operator=(const float V) POKEBOT_NOEXCEPT {
				value = std::fmod(V, 360.0f);
			}

			VecElem& operator=(const VecElem& V) POKEBOT_NOEXCEPT {
				operator=(V.value);
			}

			operator float() const POKEBOT_NOEXCEPT {
				return value;
			}
		};

		template<typename ReturnType, typename EnumType>
		inline constexpr ReturnType ToBit(const EnumType E) POKEBOT_NOEXCEPT {
			return 1 << static_cast<ReturnType>(E);
		}

		// simple non-copying base class
		class DenyCopying {
		protected:
			explicit DenyCopying() = default;
			~DenyCopying() = default;

		public:
			DenyCopying(const DenyCopying&) = delete;
			DenyCopying& operator = (const DenyCopying&) = delete;
		};

		// singleton for objects
		template<typename T> class Singleton : public DenyCopying {
		protected:
			Singleton() {
			}

		public:
			static T& Instance() {
				static T __instance{};
				return __instance;
			}

		public:
			T* operator -> () {
				return &Instance();
			}
		};
	}
}

