#include <sstream>
#include "plugin.hpp"

namespace pokebot::game {
	void OriginToAngle(Vector* destination, const Vector& Target, const Vector& Origin) {
		// float vecout[3]{};
		Vector angle = Target - Origin;
		VEC_TO_ANGLES(angle, *destination);
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
		WRITE_SHORT(pokebot::plugin::pokebot_plugin.BeamSprite());
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