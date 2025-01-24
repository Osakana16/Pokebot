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

namespace pokebot{
	namespace common {
#ifndef NDEBUG
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

		template<typename T> struct Point { T x{}, y{}, z{}; };

		template<typename T>
		concept XY = requires(T & v) {
			v.x;
			v.y;
		};

		template<typename T>
		concept XYZ = requires(T &v) {
			v.x;
			v.y;
			v.z;
		};

		template<typename ReturnType, typename EnumType>
		inline constexpr ReturnType ToBit(const EnumType E) POKEBOT_NOEXCEPT {
			return 1 << static_cast<ReturnType>(E);
		}

		template<typename T>
		class Vec {
			Point<T> point{};
		public:
			T& x = point.x;
			T& y = point.y;
			T& z = point.z;

			Vec() POKEBOT_NOEXCEPT : Vec(0, 0, 0) {}
			Vec(const T V[3]) POKEBOT_NOEXCEPT : Vec(V[0], V[1], V[2]) {}
			Vec(const T x, const T y, const T z) POKEBOT_NOEXCEPT : Vec(Point<T>{ x, y, z }) {}
			template<XYZ ElementClass> Vec(ElementClass p) POKEBOT_NOEXCEPT { x = p.x; y = p.y; z = p.z; }

			template<typename Castable>
			operator Castable() const POKEBOT_NOEXCEPT {
				Castable casted{};
				casted.x = x; casted.y = y; casted.z = z;
				return casted;
			}

			operator const float*() const POKEBOT_NOEXCEPT {
				return &x;
			}

			template<XYZ ElementClass>
			Vec<T>& operator=(const ElementClass& V) POKEBOT_NOEXCEPT {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			template<XYZ ElementClass>
			Vec<T> operator+(const ElementClass& V) const POKEBOT_NOEXCEPT {
				return Vec<T>(x + V.x, y + V.y, z + V.z);
			}

			template<XYZ ElementClass>
			Vec<T> operator-(const ElementClass& V) const POKEBOT_NOEXCEPT {
				return Vec<T>(x - V.x, y - V.y, z - V.z);
			}

			template<typename Numeric>
			Vec<T> operator*(const Numeric V) const POKEBOT_NOEXCEPT {
				static_assert(std::is_arithmetic_v<Numeric>);
				return Vec<T>(x * V, y * V, z * V);
			}

			template<typename Numeric>
			Vec<T> operator/(const Numeric V) const POKEBOT_NOEXCEPT {
				static_assert(std::is_arithmetic_v<Numeric>);
				return Vec<T>(x / V, y / V, z / V);
			}

			template<XYZ ElementClass>
			Vec<T>& operator+=(const ElementClass& V) POKEBOT_NOEXCEPT {
				x += V.x; y += V.y; z += V.z;
				return *this;
			}

			template<XYZ ElementClass>
			Vec<T>& operator-=(const ElementClass& V) POKEBOT_NOEXCEPT {
				x -= V.x; y -= V.y; z -= V.z;
				return *this;
			}

			template<XYZ Numeric>
			Vec<T>& operator*=(const Numeric V) POKEBOT_NOEXCEPT {
				static_assert(std::is_arithmetic_v<Numeric>);
				x *= V; y *= V; z *= V;
				return *this;
			}

			template<XYZ Numeric>
			Vec<T>& operator/=(const Numeric V) POKEBOT_NOEXCEPT {
				static_assert(std::is_arithmetic_v<Numeric>);
				x /= V; y /= V; z /= V;
				return *this;
			}

			T& operator[](const int I) {
				switch (I) {
					case 0:
						return point.x;
					case 1:
						return point.y;
					case 2:
						return point.z;
					default:
						assert(0);
				}
			}
		};

		class PositionVector;
		class AngleVector;

		class PositionVector final : public Vec<float> {
		public:
			using Vec<float>::Vec;
			using Vec<float>::operator=;

			PositionVector& operator=(const Vec<float>& V) POKEBOT_NOEXCEPT {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			PositionVector& operator=(const PositionVector& V) POKEBOT_NOEXCEPT {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			PositionVector& operator=(const AngleVector& V) = delete;

			AngleVector ToAngleVector(const Vector& Origin) const POKEBOT_NOEXCEPT;
		};

		class AngleVector final : public Vec<float> {
		public:
			using Vec<float>::Vec;
			using Vec<float>::operator=;

			inline AngleVector& operator=(const Vec<float>& V) POKEBOT_NOEXCEPT {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			AngleVector& operator=(const AngleVector& V) POKEBOT_NOEXCEPT {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			AngleVector& operator=(const PositionVector& V) = delete;

		};

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

		template<XYZ V>
		float Length(const V& Source) POKEBOT_NOEXCEPT {
			return std::sqrt((Source.x * Source.x) + (Source.y * Source.y) + (Source.z * Source.z));
		}

		template<XY V>
		float Length2D(const V& Source) POKEBOT_NOEXCEPT {
			return std::sqrt((Source.x * Source.x) + (Source.y * Source.y));
		}

		template<XYZ V1, XYZ V2>
		float Distance(const V1& S1, const V2& S2) POKEBOT_NOEXCEPT {
			return Length(S1 - S2);
		}


		template<XYZ V1, XYZ V2>
		float Distance2D(const V1& S1, const V2& S2) POKEBOT_NOEXCEPT {
			return Length2D(S1 - S2);
		}

		template<XYZ V>
		std::optional<V> Normalize(const V& Source) POKEBOT_NOEXCEPT {
			V result{};
			float flLen = Length(Source);
			if (flLen == 0) return std::nullopt;
			flLen = 1 / flLen;
			result.x = Source.x * flLen;
			result.y = Source.y * flLen;
			result.z = Source.z * flLen;
			return result;
		}


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