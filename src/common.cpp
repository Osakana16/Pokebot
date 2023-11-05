#include <sstream>

extern int Beam_Sprite;

namespace pokebot::common {
	AngleVector PositionVector::ToAngleVector(const Vector& Origin) const noexcept {
		float vecout[3]{};
		Vector angle = *this - Origin;
		VEC_TO_ANGLES(angle, vecout);
		return AngleVector(vecout);
	}


	Tracer& Tracer::MoveStart(const Vector& Start) {
		start_position = Start;
		return *this;
	}

	Tracer& Tracer::MoveDest(const Vector& Dest) {
		dest_position = Dest;
		return *this;
	}

	Tracer& Tracer::TraceHull(Monsters monsters, HullType hull, edict_t* ignore_entity) {
		TRACE_HULL(start_position, dest_position, static_cast<IGNORE_MONSTERS>(monsters), static_cast<HULL_TYPE>(hull), ignore_entity, this);
		return *this;
	}

	Tracer& Tracer::TraceLine(Monsters monsters, Glass glass, edict_t* ignore_entity) {
		TRACE_LINE(start_position, dest_position, static_cast<IGNORE_MONSTERS>(monsters) | static_cast<IGNORE_GLASS>(glass), ignore_entity, this);
		return *this;
	}

	bool Tracer::IsHit() const noexcept {
		return flFraction < 1.0;
	}

	void Draw(edict_t* ent, const Vector& start, const Vector& end, int width, int noise, const Color& color, int brightness, int speed, int life) {
		MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, nullptr, ent);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(end.x);
		WRITE_COORD(end.y);
		WRITE_COORD(end.z);
		WRITE_COORD(start.x);
		WRITE_COORD(start.y);
		WRITE_COORD(start.z);
		WRITE_SHORT(Beam_Sprite);
		WRITE_BYTE(0); // framestart
		WRITE_BYTE(10); // framerate
		WRITE_BYTE(life); // life in 0.1's
		WRITE_BYTE(width); // width
		WRITE_BYTE(noise); // noise
		WRITE_BYTE(color.r); // r, g, b
		WRITE_BYTE(color.g); // r, g, b
		WRITE_BYTE(color.b); // r, g, b
		WRITE_BYTE(brightness); // brightness
		WRITE_BYTE(speed); // speed
		MESSAGE_END();
	}
}