export module pokebot.util.tracer;
#include "vector.h"

namespace pokebot::util {
	export class Tracer final : public TraceResult {
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

		Tracer& MoveStart(const Vector& Start) {
			start_position = Start;
			return *this;
		}

		Tracer& MoveDest(const Vector& Dest) {
			dest_position = Dest;
			return *this;
		}

		Tracer& TraceHull(Monsters monsters, HullType hull, edict_t* ignore_entity) noexcept {
			TRACE_HULL(start_position, dest_position, static_cast<IGNORE_MONSTERS>(monsters), static_cast<HULL_TYPE>(hull), ignore_entity, this);
			return *this;
		}

		Tracer& TraceLine(Monsters monsters, edict_t* ignore_entity) noexcept {
			TRACE_LINE(start_position, dest_position, static_cast<IGNORE_MONSTERS>(monsters), ignore_entity, this);
			return *this;
		}

		bool IsHit() const noexcept {
			return flFraction < 1.0;
		}
	};
}
