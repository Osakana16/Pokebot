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
		class Tracer final : public TraceResult {
			Vector start_position{}, dest_position{};
		public:
			enum class Monsters {
				Ignore = IGNORE_MONSTERS::ignore_monsters,
				Dont_Ignore = IGNORE_MONSTERS::dont_ignore_monsters,
				Missile = IGNORE_MONSTERS::missile
			};

			enum class HullType {
				Point = HULL_TYPE::point_hull,
				Head = HULL_TYPE::head_hull,
				Human = HULL_TYPE::human_hull,
				Large = HULL_TYPE::large_hull
			};

			enum class Glass {
				Ignore = IGNORE_GLASS::ignore_glass,
				Dont_Ignore = IGNORE_GLASS::dont_ignore_glass
			};
		public:
			Tracer() = default;

			Tracer& MoveStart(const Vector& Start);
			Tracer& MoveDest(const Vector& Dest);
			Tracer& TraceHull(Monsters monsters, HullType hull, edict_t* ignore_entity);
			Tracer& TraceLine(Monsters monsters, Glass glass, edict_t* ignore_entity);
			bool Tracer::IsHit() const POKEBOT_NOEXCEPT;
		};

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

		template<typename T>
		class Random {};

		template<>
		class Random<int> {
			std::random_device device{};
			std::mt19937 mt;
			std::uniform_int_distribution<int> distribution;
		public:
			Random(const int min, const int max) : mt(device()), distribution(min, max) {}

			template<typename T>
			operator T() { return distribution(mt); }
		};

		template<>
		class Random<float> {
			std::random_device device{};
			std::mt19937 mt;
			std::uniform_real_distribution<float> distribution;
		public:
			Random(const float min, const float max) : mt(device()), distribution(min, max) {}

			template<typename T>
			operator T() { return distribution(mt); }
		};

		template<size_t N>
		class fixed_string {
			static_assert(N >= 2);
			char str[N]{};
		public:
			fixed_string() {}
			fixed_string(const char* const a) { operator=(a); }

			char* begin() noexcept { return &str[0]; }
			const char* cbegin() const noexcept { return &str[0]; }
			char* end() noexcept { return &str[N - 1]; }
			const char* cend() const noexcept { return &str[N - 1]; }
			void clear() noexcept { memset(str, '\0', N); }
			
			template<size_t M> void push_back(const fixed_string<M>& a) noexcept { operator=(a); }
			void push_back(const char* const a) noexcept { operator+=(a); }
			void push_back(const char a) noexcept { operator+=(a); }
			bool contain(const char* const a) const noexcept { return strstr(str, a) != nullptr; }
			const char* c_str() const noexcept { return str; }
			char* data() noexcept { return str; }
			const char* data() const noexcept { return str; }
			bool empty() const noexcept { return strlen(str) <= 0; }

			static consteval size_t size() noexcept { return N; }
			
			template<size_t M> bool operator==(const fixed_string<M>& a) const noexcept { return operator==(a.c_str()); }
			template<size_t M> bool operator!=(const fixed_string& a) const noexcept { return operator!=(a.c_str()); }
			bool operator==(const char* const a) const noexcept { return strcmp(str, a) == 0; }
			bool operator!=(const char* const a) const noexcept { return !operator==(a); }
		
			char operator[](const int i) noexcept { return str[i]; }
			char operator[](const int i) const noexcept { return str[i]; }
			char at(const int i) noexcept { assert(i >= 0 && i < N); return str[i]; }
			char at(const int i) const noexcept { assert(i >= 0 && i < N); return str[i]; }

			template<size_t M>
			fixed_string& operator=(const fixed_string<M>& a) noexcept {
				static_assert(M <= N);
				return operator=(a.c_str());
			}

			fixed_string& operator=(const char* const a) noexcept {
				assert(strlen(a) <= N);
				strcpy(str, a);
				return *this;
			}

			fixed_string& operator=(const char a) noexcept {
				const char res[2]{ a, '\0' };
				return operator=(res);
			}

			template<size_t M>
			fixed_string& operator+=(const fixed_string<M>& a) noexcept {
				static_assert(M <= N);
				return operator+=(a.c_str());
			}

			fixed_string& operator+=(const char* const a) noexcept {
				assert(strlen(str) + strlen(a) <= N);
				strcat(str, a);
				return *this;
			}

			fixed_string& operator+=(const char a) noexcept {
				const char res[2]{ a, '\0' };
				return operator+=(res);
			}
			
			template<size_t M>
			fixed_string operator+(const fixed_string<M>& a) const noexcept {
				static_assert(M <= N);
				return operator+(a.c_str());
			}

			fixed_string operator+(const char* const a) const noexcept {
				char res[N]{};
				assert(strlen(str) + strlen(a) <= N);
				strcat(res, str);
				strcat(res, a);
				return fixed_string<N>{res};
			}

			fixed_string operator+(const char a) const noexcept {
				char res[N]{};
				char a_to_str[2]{ a, '\0' }; 
				return operator+(a_to_str);
			}

			struct Hash {
				inline size_t operator()(const fixed_string& s) const {
					return std::hash<std::string>()(s.c_str());
				}
			};
		};

		// Maximum player length is 32.
		using PlayerName = fixed_string<32u>;

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

#include "graph.hpp"
#include "game.hpp"
#include "bot/basic.hpp"
#include "bot/bot.hpp"