export module pokebot.game.util;

export namespace pokebot::game {
    float Distance(const Vector& S1, const Vector& S2) POKEBOT_NOEXCEPT { return (S1 - S2).Length(); }
    float Distance2D(const Vector& S1, const Vector& S2) POKEBOT_NOEXCEPT { return (S1 - S2).Length2D(); }

    edict_t* FindEntityInSphere(edict_t* pentStart, const Vector& vecCenter, float flRadius) {
        edict_t* pentEntity = FIND_ENTITY_IN_SPHERE(pentStart, vecCenter, flRadius);
        if (!FNullEnt(pentEntity))
            return pentEntity;

        return NULL;
    }

    edict_t* FindEntityByString(edict_t* pentStart, const char* szKeyword, const char* szValue) {
        edict_t* pentEntity = FIND_ENTITY_BY_STRING(pentStart, szKeyword, szValue);
        if (!FNullEnt(pentEntity))
            return pentEntity;
        return NULL;
    }

    edict_t* FindEntityByClassname(edict_t* pentStart, const char* szName) {
        return FindEntityByString(pentStart, "classname", szName);
    }

    edict_t* FindEntityByTargetname(edict_t* pentStart, const char* szName) {
        return FindEntityByString(pentStart, "targetname", szName);
    }

    Vector VecBModelOrigin(edict_t* pEdict) {
        return pEdict->v.absmin + (pEdict->v.size * 0.5);
    }

    Vector UTIL_VecToAngles(const Vector& vec) {
        float rgflVecOut[3];
        VEC_TO_ANGLES(vec, rgflVecOut);
        return Vector(rgflVecOut);
    }

    game::Team GetTeamFromModel(const edict_t* const Edict) {
        static std::unordered_map<pokebot::util::fixed_string<11u>, game::Team, pokebot::util::fixed_string<11u>::Hash> Model_And_Teams{
            { "terror", game::Team::T },
            { "arab", game::Team::T },
            { "leet", game::Team::T },
            { "artic", game::Team::T },
            { "arctic", game::Team::T },
            { "guerilla", game::Team::T },
            { "urban", game::Team::CT },
            { "gsg9", game::Team::CT },
            { "sas", game::Team::CT },
            { "gign", game::Team::CT },
            { "vip", game::Team::CT },
            { "spetsnatz", game::Team::CT }
        };
        auto infobuffer = (*g_engfuncs.pfnGetInfoKeyBuffer)(const_cast<edict_t*>(Edict));
        return Model_And_Teams.at((*g_engfuncs.pfnInfoKeyValue)(infobuffer, "model"));
    }

    void OriginToAngle(Vector* destination, const Vector& Target, const Vector& Origin) {
        // float vecout[3]{};
        Vector angle = Target - Origin;
        VEC_TO_ANGLES(angle, *destination);
    }
}