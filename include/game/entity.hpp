#pragma once
#include "team.hpp"
namespace pokebot::game {
	inline float Distance(const Vector& S1, const Vector& S2) POKEBOT_NOEXCEPT { return (S1 - S2).Length(); }
	inline float Distance2D(const Vector& S1, const Vector& S2) POKEBOT_NOEXCEPT { return (S1 - S2).Length2D(); }

	void OriginToAngle(Vector* destination, const Vector& Target, const Vector& Origin);
	edict_t* FindEntityInSphere(edict_t* pentStart, const Vector& vecCenter, float flRadius);
	edict_t* FindEntityByString(edict_t* pentStart, const char* szKeyword, const char* szValue);
	edict_t* FindEntityByClassname(edict_t* pentStart, const char* szName);
	edict_t* FindEntityByTargetname(edict_t* pentStart, const char* szName);
	Vector VecBModelOrigin(edict_t* pEdict);
	Vector UTIL_VecToAngles(const Vector& vec);
	game::Team GetTeamFromModel(const edict_t* const);

	void Draw(edict_t* ent, const Vector& start, const Vector& end, int width, int noise, const game::Color& color, int brightness, int speed, int life);
}