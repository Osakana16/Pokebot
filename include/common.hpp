#pragma once
#include <sstream>

#define POKEBOT_ENUM_BIT_OPERATORS(TYPE) inline constexpr TYPE operator|(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 | (int)n2); } inline constexpr TYPE operator&(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 & (int)n2); } inline constexpr TYPE operator^(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 ^ (int)n2); } inline constexpr TYPE operator|=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 | n2); return n1; } inline constexpr TYPE operator&=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 & n2); return n1; } inline constexpr TYPE operator^=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 ^ n2); return n1; } inline constexpr TYPE operator~(TYPE n1) noexcept { return static_cast<TYPE>(~static_cast<int>(n1)); }
#define POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(TYPE,...) enum class TYPE { __VA_ARGS__ }; POKEBOT_ENUM_BIT_OPERATORS(TYPE)


#define POKEBOT_DEBUG_NOEXCEPT noexcept(false)
#define DEBUG_PRINTF(...) SERVER_PRINT(std::format(__VA_ARGS__).c_str())

namespace pokebot{
	namespace common {
		using Dec = float;
		using Time = Dec;

		inline std::string ToString(int i) noexcept {
			return STRING(i);
		}

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

			VecElem& operator=(const float V) noexcept {
				value = std::fmod(V, 360.0f);
			}

			VecElem& operator=(const VecElem& V) noexcept {
				operator=(V.value);
			}

			operator float() const noexcept {
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
		inline constexpr ReturnType ToBit(const EnumType E) noexcept {
			return 1 << static_cast<ReturnType>(E);
		}

		template<typename T>
		class Vec {
			Point<T> point{};
		public:
			T& x = point.x;
			T& y = point.y;
			T& z = point.z;

			Vec() noexcept : Vec(0, 0, 0) {}
			Vec(const T V[3]) noexcept : Vec(V[0], V[1], V[2]) {}
			Vec(const T x, const T y, const T z) noexcept : Vec(Point<T>{ x, y, z }) {}
			template<XYZ ElementClass> Vec(ElementClass p) noexcept { x = p.x; y = p.y; z = p.z; }

			template<typename Castable>
			operator Castable() const noexcept {
				Castable casted{};
				casted.x = x; casted.y = y; casted.z = z;
				return casted;
			}

			operator const float*() const noexcept {
				return &x;
			}

			template<XYZ ElementClass>
			Vec<T>& operator=(const ElementClass& V) noexcept {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			template<XYZ ElementClass>
			Vec<T> operator+(const ElementClass& V) const noexcept {
				return Vec<T>(x + V.x, y + V.y, z + V.z);
			}

			template<XYZ ElementClass>
			Vec<T> operator-(const ElementClass& V) const noexcept {
				return Vec<T>(x - V.x, y - V.y, z - V.z);
			}

			template<typename Numeric>
			Vec<T> operator*(const Numeric V) const noexcept {
				static_assert(std::is_arithmetic_v<Numeric>);
				return Vec<T>(x * V, y * V, z * V);
			}

			template<typename Numeric>
			Vec<T> operator/(const Numeric V) const noexcept {
				static_assert(std::is_arithmetic_v<Numeric>);
				return Vec<T>(x / V, y / V, z / V);
			}

			template<XYZ ElementClass>
			Vec<T>& operator+=(const ElementClass& V) noexcept {
				x += V.x; y += V.y; z += V.z;
				return *this;
			}

			template<XYZ ElementClass>
			Vec<T>& operator-=(const ElementClass& V) noexcept {
				x -= V.x; y -= V.y; z -= V.z;
				return *this;
			}

			template<XYZ Numeric>
			Vec<T>& operator*=(const Numeric V) noexcept {
				static_assert(std::is_arithmetic_v<Numeric>);
				x *= V; y *= V; z *= V;
				return *this;
			}

			template<XYZ Numeric>
			Vec<T>& operator/=(const Numeric V) noexcept {
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

			PositionVector& operator=(const Vec<float>& V) noexcept {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			PositionVector& operator=(const PositionVector& V) noexcept {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			PositionVector& operator=(const AngleVector& V) = delete;

			AngleVector ToAngleVector(const Vector& Origin) const noexcept;
		};

		class AngleVector final : public Vec<float> {
		public:
			using Vec<float>::Vec;
			using Vec<float>::operator=;

			inline AngleVector& operator=(const Vec<float>& V) noexcept {
				x = V.x; y = V.y; z = V.z;
				return *this;
			}

			AngleVector& operator=(const AngleVector& V) noexcept {
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
			bool Tracer::IsHit() const noexcept;
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

		inline std::vector<std::string> StringSplit(const std::string* Target, char trim) {
			std::stringstream stream{ *Target };
			std::vector<std::string> params{};
			std::string buffer{};
			while (std::getline(stream, buffer, trim)) {
				params.push_back(std::move(buffer));
			}
			return params;
		}

		template<XYZ V>
		float Length(const V& Source) noexcept {
			return std::sqrt((Source.x * Source.x) + (Source.y * Source.y) + (Source.z * Source.z));
		}

		template<XY V>
		float Length2D(const V& Source) noexcept {
			return std::sqrt((Source.x * Source.x) + (Source.y * Source.y));
		}

		template<XYZ V1, XYZ V2>
		float Distance(const V1& S1, const V2& S2) noexcept {
			return Length(S1 - S2);
		}


		template<XYZ V1, XYZ V2>
		float Distance2D(const V1& S1, const V2& S2) noexcept {
			return Length2D(S1 - S2);
		}

		template<XYZ V>
		std::optional<V> Normalize(const V& Source) noexcept {
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
#include "bot.hpp"