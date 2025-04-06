#pragma once
#include <sstream>

#define POKEBOT_ENUM_BIT_OPERATORS(TYPE) inline constexpr TYPE operator|(const TYPE& n1, const TYPE& n2) POKEBOT_NOEXCEPT { return static_cast<TYPE>((int)n1 | (int)n2); } inline constexpr TYPE operator&(const TYPE& n1, const TYPE& n2) POKEBOT_NOEXCEPT { return static_cast<TYPE>((int)n1 & (int)n2); } inline constexpr TYPE operator^(const TYPE& n1, const TYPE& n2) POKEBOT_NOEXCEPT { return static_cast<TYPE>((int)n1 ^ (int)n2); } inline constexpr TYPE operator|=(TYPE& n1, const TYPE& n2) POKEBOT_NOEXCEPT { n1 = (n1 | n2); return n1; } inline constexpr TYPE operator&=(TYPE& n1, const TYPE& n2) POKEBOT_NOEXCEPT { n1 = (n1 & n2); return n1; } inline constexpr TYPE operator^=(TYPE& n1, const TYPE& n2) POKEBOT_NOEXCEPT { n1 = (n1 ^ n2); return n1; } inline constexpr TYPE operator~(TYPE n1) POKEBOT_NOEXCEPT { return static_cast<TYPE>(~static_cast<int>(n1)); }
#define POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(TYPE,...) enum class TYPE { __VA_ARGS__ }; POKEBOT_ENUM_BIT_OPERATORS(TYPE)

#ifndef NDEBUG
#define POKEBOT_NOEXCEPT noexcept(false)
#else
#define POKEBOT_NOEXCEPT noexcept(true)
#endif
#define DEBUG_PRINTF(...) SERVER_PRINT(std::format(__VA_ARGS__).c_str())

#define USE_STDARRAY false

namespace pokebot{
	namespace common {
#ifndef USE_STDARRAY
		template<typename T, size_t N>
		using Array = std::array<T, N>;
#else
		template<typename T, size_t N>
		using Array = T[N];
#endif

		using Dec = float;
		using Time = Dec;

		constexpr auto Third_Party_Bot_Flag = 1 << 27;

		enum class Team {
			Spector,
			T = 1,
			CT,
			Random
		};

		struct Color {
			int r{}, g{}, b{};
		};

		enum class Model {
			Phoenix_Connexion = 1,
			Elite_Crew,
			Artic_Avengers,
			Guerilla_Warface,
			Midwest_Militia,
			SEAL = 1,
			GSG9,
			SAS,
			GIGN,
			Spetsnaz,
			Random = 6
		};

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

		inline float Distance(const Vector& S1, const Vector& S2) POKEBOT_NOEXCEPT { return (S1 - S2).Length(); }
		inline float Distance2D(const Vector& S1, const Vector& S2) POKEBOT_NOEXCEPT { return (S1 - S2).Length2D(); }

		void OriginToAngle(Vector* destination, const Vector& Target, const Vector& Origin);
		edict_t* FindEntityInSphere(edict_t* pentStart, const Vector& vecCenter, float flRadius);
		edict_t* FindEntityByString(edict_t* pentStart, const char* szKeyword, const char* szValue);
		edict_t* FindEntityByClassname(edict_t* pentStart, const char* szName);
		edict_t* FindEntityByTargetname(edict_t* pentStart, const char* szName);
		Vector VecBModelOrigin(edict_t* pEdict);
		Vector UTIL_VecToAngles(const Vector& vec);
		Team GetTeamFromModel(const edict_t* const);

		void Draw(edict_t* ent, const Vector& start, const Vector& end, int width, int noise, const Color& color, int brightness, int speed, int life);
	}
}

#include "util/fixed_string.hpp"

#include "graph.hpp"
#include "game.hpp"
#include "bot/basic.hpp"
#include "bot/bot.hpp"