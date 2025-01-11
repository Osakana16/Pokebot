#include "common.hpp"

namespace {
    int current_message{};

    std::vector<std::variant<int, float, std::string>> args{};
}

const edict_t* engine_target_edict = nullptr;
bool is_bot{};
bool is_host{};

namespace pokebot::common {
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

    Team GetTeamFromModel(const edict_t* const Edict) {
        auto infobuffer = (*g_engfuncs.pfnGetInfoKeyBuffer)(const_cast<edict_t*>(Edict));
        std::string model_name = (*g_engfuncs.pfnInfoKeyValue)(infobuffer, "model");

        if (model_name == "terror" ||        // Phoenix Connektion
            model_name == "arab" ||  // old L337 Krew
            model_name == "leet" ||  // L337 Krew
            model_name == "artic" || // Artic Avenger
            model_name == "arctic" ||        // Artic Avenger - fix for arctic? - seemed a typo?
            model_name == "guerilla")        // Gorilla Warfare
        {
            return Team::T; // team Terrorists
        } else if (model_name == "urban" ||  // Seal Team 6
            model_name == "gsg9" ||   // German GSG-9
            model_name == "sas" ||    // UK SAS
            model_name == "gign" ||   // French GIGN
            model_name == "vip" ||    // VIP
            model_name == "spetsnatz")        // CZ Spetsnatz
        {
            return Team::CT; // team Counter-Terrorists
        }
        return Team::Random;
    }
}

void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, edict_t* pentIgnore, TraceResult* ptr) {
    TRACE_LINE(vecStart, vecEnd, igmon, pentIgnore, ptr);
}


void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS iglass, edict_t* pentIgnore, TraceResult* ptr) {
    TRACE_LINE(vecStart, vecEnd, igmon | iglass, pentIgnore, ptr);
}

void UTIL_TraceHull(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr) {
    TRACE_HULL(vecStart, vecEnd, igmon, hullNumber, pentIgnore, ptr);
}


C_DLLEXPORT int
GetEngineFunctions(enginefuncs_t* pengfuncsFromEngine, int* interfaceVersion) {
    meta_engfuncs.pfnMessageBegin = [](int msg_dest, int msg_type, const float* pOrigin, edict_t* edict) {
        if (gpGlobals->deathmatch) {
            using namespace pokebot;
            is_bot = (edict != nullptr && pokebot::bot::Manager::Instance().IsExist(STRING(edict->v.netname)));
            is_host = (edict == game::game.host.AsEdict());
        
            engine_target_edict = edict;
            current_message = msg_type;
        }

        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnMessageEnd = []() {        
        if (gpGlobals->deathmatch) {
            static std::unordered_map<int, std::function<void()>> messages
            {
                {
                    GET_USER_MSG_ID(PLID, "VGUIMenu", nullptr), []() {
                        if (is_host) {
                            return;
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "BotVoice", nullptr), []() {
                        if (is_host) {
                            return;
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "ShowMenu", nullptr), []() POKEBOT_NOEXCEPT {
                        if (!is_bot) {
                            return;
                        }

                        if (is_host) {
                            pokebot::game::game.RegisterClient(const_cast<edict_t*>(engine_target_edict));
                        }

                        if (args.size() >= 4) {
                            static const std::unordered_map<std::string, pokebot::bot::Message> Menu_Cache {
                                { "#Team_Select", pokebot::bot::Message::Team_Select },
                                { "#Team_Select_Spect", pokebot::bot::Message::Team_Select },
                                { "#IG_Team_Select", pokebot::bot::Message::Team_Select },
                                { "#IG_Team_Select_Spect", pokebot::bot::Message::Team_Select },
                                { "#IG_VIP_Team_Select", pokebot::bot::Message::Team_Select },
                                { "#IG_VIP_Team_Select_Spect", pokebot::bot::Message::Team_Select },
                                { "#Terrorist_Select", pokebot::bot::Message::Model_Select },
                                { "#CT_Select", pokebot::bot::Message::Model_Select }
                            };

                            auto it = Menu_Cache.find(std::get<std::string>(args[3]));
                            if (it != Menu_Cache.end()) {
                                pokebot::bot::Manager::Instance().Assign(STRING(engine_target_edict->v.netname), it->second);
                            }
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "WeaponList", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() < 9)
                            return;


                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "TeamInfo", nullptr), []() {
                        if (engine_target_edict == nullptr || args.size() < 2)
                            return;

                        using namespace pokebot;
                        static const std::unordered_map<std::string, pokebot::common::Team> Menu_Cache {
                            { "TERRORIST", common::Team::T },
                            { "CT", common::Team::CT },
                            { "UNASSIGNED", common::Team::Spector },
                            { "SPECTATOR", common::Team::Spector }
                        };

                        if (Menu_Cache.at(std::get<std::string>(args[1])) == common::Team::Spector) {
                            game::game.RegisterClient(const_cast<edict_t*>(engine_target_edict));
                        } else {
                            game::game.OnTeamAssigned(STRING(engine_target_edict->v.netname), Menu_Cache.at(std::get<std::string>(args[1])));
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "ScoreInfo", nullptr), []() {
                        if (args.size() < 5)
                            return;


                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "ScoreAttrib", nullptr), []() {
                        if (args.size() < 2)
                            return;

                        enum ScoreStatus : int {
                            Nothing = 0,
                            Dead = 1 << 0,
                            Bomb = 1 << 1,
                            VIP = 1 << 2,
                            Defuse_Kit = 1 << 3
                        };

                        if (bool(std::get<int>(args[1]) & ScoreStatus::VIP)) {
                            pokebot::game::game.OnVIPChanged(STRING(INDEXENT(std::get<int>(args[0]))->v.netname));
                        } else if ((std::get<int>(args[1]) & ScoreStatus::Dead)) {

                        } else if ((std::get<int>(args[1]) & ScoreStatus::Defuse_Kit)) {

                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "CurWeapon", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() < 3 || !std::holds_alternative<int>(args[1]))
                            return;

                        if (std::get<int>(args[1]) < 32) {
                            using namespace pokebot;
                            if (std::get<int>(args[0]) != 0) {
                                game::game.OnWeaponChanged(STRING(engine_target_edict->v.netname), (game::Weapon)std::get<int>(args[1]));
                            }
                            game::game.OnClipChanged(STRING(engine_target_edict->v.netname), static_cast<game::Weapon>(std::get<int>(args[1])), std::get<int>(args[2]));
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "AmmoX", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        messages.at(GET_USER_MSG_ID(PLID, "AmmoPickup", nullptr))();
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "AmmoPickup", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() < 2) {
                            return;
                        }
                        const int Ammo_ID = std::get<int>(args[0]);
                        const int Amount = std::get<int>(args[1]);
                        if (Ammo_ID <= 0 || Ammo_ID > 10)
                            return;

                        using namespace pokebot;
                        game::game.OnAmmoPickedup(STRING(engine_target_edict->v.netname), static_cast<game::AmmoID>(Ammo_ID - 1), Amount);
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "Money", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() >= 1 && std::holds_alternative<int>(args[0])) {
                            using namespace pokebot;
                            game::game.OnMoneyChanged(STRING(engine_target_edict->v.netname), std::clamp(std::get<int>(args[0]), 0, 16000));
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "Damage", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        // check the minimum states
                        if (args.size() < 3 || !std::holds_alternative<int>(args[0]) || !std::holds_alternative<int>(args[1]) || !std::holds_alternative<int>(args[2])) {
                           return;
                        }
                    
                        using namespace pokebot;

                        const int Health = std::get<int>(args[1]);
                        const int Armor = std::get<int>(args[0]);
                        const int Bit = std::get<int>(args[2]);
                        game::game.OnDamageTaken(STRING(engine_target_edict->v.netname), engine_target_edict->v.dmg_inflictor, Health, Armor, Bit);
                        bot::Manager::Instance().OnDamageTaken(STRING(engine_target_edict->v.netname), engine_target_edict->v.dmg_inflictor,  Health, Armor, Bit);
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "StatusIcon", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() < 2 || !std::holds_alternative<int>(args[0]) || !std::holds_alternative<std::string>(args[1])) {
                            return;
                        }

                        using namespace pokebot::game;
                        static const std::unordered_map<std::string, StatusIcon> Icon_Cache{
                            { "c4", StatusIcon::C4 },
                            { "buyzone", StatusIcon::Buy_Zone },
                            { "rescue",  StatusIcon::Rescue_Zone },
                            { "vipsafety", StatusIcon::Vip_Safety },
                        };

                        if (auto it = Icon_Cache.find(std::get<std::string>(args[1])); it != Icon_Cache.end()) {
                            using namespace pokebot;
                            game::game.OnStatusIconShown(STRING(engine_target_edict->v.netname), it->second);
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "ItemStatus", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() < 1 || !std::holds_alternative<int>(args[0])) {
                            return;
                        }

                        using namespace pokebot;
                        game::game.OnItemChanged(STRING(engine_target_edict->v.netname), static_cast<game::Item>(std::get<int>(args[0])));
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "NVGToggle", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        // NVG Toggle message.

                        if (args.size() < 1 || !std::holds_alternative<int>(args[0])) {
                            return;
                        }

                        using namespace pokebot;
                        game::game.OnNVGToggled(STRING(engine_target_edict->v.netname), std::get<int>(args[0]) > 0);
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "FlashBat", nullptr), []() {

                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "BarTime", nullptr), []() {

                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "TextMsg", nullptr), []() {
                        auto nothingToDo = [] {};
                        auto failIfBotDoes = [] { assert(!is_bot); };

                        const std::unordered_map<std::string, std::function<void()>> Text_Message{
                            { "#Game_scoring", nothingToDo },
                            { "#Cstrike_Chat_All", nothingToDo },
                            { "#Cstrike_Chat_CT", nothingToDo },
                            { "#Cstrike_Chat_T", nothingToDo },
                            { "#CTs_Win", nothingToDo },
                            { "#Terrorists_Win", nothingToDo },
                            { "#Round_Draw", nothingToDo },
                            { "#All_Hostages_Rescued", nothingToDo },
                            { "#Target_Saved", nothingToDo },
                            { "#Bomb_Defused", nothingToDo },
                            { "#Bomb_Planted", nothingToDo },
                            { "#Hostages_Not_Rescued", nothingToDo,},
                            { "#Terrorists_Not_Escaped", nothingToDo },
                            { "#VIP_Not_Escaped", nothingToDo },
                            { "#Escaping_Terrorists_Neutralized", nothingToDo },
                            { "#Terrorist_Escaped", nothingToDo },
                            { "#VIP_Assassinated", nothingToDo },
                            { "#VIP_Escaped", nothingToDo },
                            { "#Terrorists_Escaped", nothingToDo },
                            { "#CTs_PreventEscape", nothingToDo },
                            { "#Target_Bombed", nothingToDo },
                            { "#Game_Commencing", nothingToDo },
                            { "#Game_will_restart_in", nothingToDo },
                            { "#Switch_To_BurstFire", nothingToDo },
                            { "#Switch_To_SemiAuto", nothingToDo },
                            { "#Game_radio", nothingToDo },
                            { "#Killed_Teammate", nothingToDo },
                            { "#C4_Plant_Must_Be_On_Ground", nothingToDo },
                            { "#Injured_Hostage", nothingToDo },
                            { "#Auto_Team_Balance_Next_Round", nothingToDo },
                            { "#Killed_Hostage", nothingToDo },
                            { "#Too_Many_Terrorists", nothingToDo },
                            { "#Too_Many_CTs", nothingToDo },
                            { "#Weapon_Not_Available", nothingToDo },
                            { "#Game_bomb_pickup", nothingToDo },
                            { "#Got_bomb", nothingToDo },
                            { "#Game_bomb_drop", nothingToDo },
                            { "#Not_Enough_Money", nothingToDo },
                            { "#Cstrike_Already_Own_Weapon", nothingToDo },
                            // - The message that should not be sent caused by a bot. -
                            { "#Weapon_Cannot_Be_Dropped", failIfBotDoes },
                            { "#C4_Plant_At_Bomb_Spot", failIfBotDoes },
                            { "#VIP_cant_buy", failIfBotDoes },
                            { "#Cannot_Switch_From_VIP", failIfBotDoes },
                            { "#Only_1_Team_Change", failIfBotDoes },
                            // - Spec Mode(Always nothing to do) -
                            { "#Spec_Mode1", nothingToDo },
                            { "#Spec_Mode2", nothingToDo },
                            { "#Spec_Mode3", nothingToDo },
                            { "#Spec_Mode4", nothingToDo },
                            { "#Spec_Mode5", nothingToDo },
                            { "#Spec_Mode6", nothingToDo },
                            { "#Spec_NoTarget", nothingToDo }
                        };

                        using namespace pokebot;
                        if (args.size() >= 5 && std::holds_alternative<std::string>(args[2])) {
                            auto it = Text_Message.find(std::get<std::string>(args[2]));
                            auto& sender = std::get<std::string>(args[3]);
                            auto& radio = std::get<std::string>(args[4]);

                            if (it == Text_Message.find("#Game_radio")) {
                                bot::Manager::Instance().OnRadioRecieved(sender, radio);
                            }
                        } else if (args.size() >= 3 && std::holds_alternative<std::string>(args[2])) {
                            auto it = Text_Message.find(std::get<std::string>(args[1]));
                            if (it != Text_Message.end()) {

                            }
                        } else if (args.size() >= 2 && std::holds_alternative<std::string>(args[1])) {
                            Text_Message.at(std::get<std::string>(args[1]))();
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "SayText", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.empty()) {

                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "SayTextTeam", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "ScreenFade", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        // Flashbang message.

                        if (args.size() < 7 || !std::holds_alternative<int>(args[3]) || !std::holds_alternative<int>(args[4]) || !std::holds_alternative<int>(args[5]) || !std::holds_alternative<int>(args[6]))
                            return;

                        if (std::get<int>(args[3]) >= 255 && std::get<int>(args[4]) >= 255 && std::get<int>(args[5]) >= 255 && std::get<int>(args[6]) > 170) {
                            // TODO: Blind a bot.
                        }
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "StatusValue", nullptr), []() {
                        DEBUG_PRINTF("StatusValue\n");
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "DeathMsg", nullptr), []() {
                        if (!is_bot) {
                            return;
                        }

                        if (args.size() < 2)
                            return;

                        using namespace pokebot::game;
                    }
                },
                {
                    GET_USER_MSG_ID(PLID, "HLTV", nullptr), []() POKEBOT_NOEXCEPT {
                        // New round message in CS1.6.

                        if (args.size() < 2 || !std::holds_alternative<int>(args[0]) || !std::holds_alternative<int>(args[1])) {
                            return;
                        }

                        if (std::get<int>(args[0]) == 0 && std::get<int>(args[1]) == 0) {
                            pokebot::game::game.OnNewRound();
                        }
                    }
                }
            };

            auto it = messages.find(current_message);
            if (it != messages.end()) {
                it->second();
            }
            args.clear();
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnChangeLevel = [](const char* s1, const char* s2) {
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnFindEntityByString = [](edict_t* pEdictStartSearchAfter, const char* pszField, const char* pszValue) -> edict_t* {
        if (gpGlobals->deathmatch) {
            std::string string_value = pszValue;
            if (string_value == "info_map_parameters") {
                pokebot::game::game.OnNewRound();
            }
        }
        RETURN_META_VALUE(MRES_IGNORED, nullptr);
    };

    meta_engfuncs.pfnRemoveEntity = [](edict_t* e) {
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnClientCommand = [](edict_t* pEdict, const char* szFmt, ...) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            if (pEdict->v.flags & (FL_FAKECLIENT | pokebot::common::Third_Party_Bot_Flag))
                RETURN_META(MRES_SUPERCEDE);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteByte = [](int value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteChar = [](int value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteShort = [](int value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteLong = [](int value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteAngle = [](float value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteCoord = [](float value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteString = [](const char* value) POKEBOT_NOEXCEPT {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnWriteEntity = [](int value) {
        if (gpGlobals->deathmatch) {
            args.push_back(value);
        }
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnClientPrintf = [](edict_t* pEdict, PRINT_TYPE ptype, const char* szMsg) {
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnCmd_Args = []() -> const char* {
        if (pokebot::game::game.IsBotCmd())
            RETURN_META_VALUE(MRES_SUPERCEDE, pokebot::game::game.GetBotArg(0).c_str()); // returns the wanted argument

        RETURN_META_VALUE(MRES_IGNORED, nullptr);
    };

    meta_engfuncs.pfnCmd_Argv = [](int argc) -> const char* {
        if (pokebot::game::game.IsBotCmd()) {
            RETURN_META_VALUE(MRES_SUPERCEDE, pokebot::game::game.GetBotArg(argc).c_str()); // returns the wanted argument
        }
        RETURN_META_VALUE(MRES_IGNORED, nullptr);
    };

    meta_engfuncs.pfnCmd_Argc = []() -> int {
        if (pokebot::game::game.IsBotCmd()) {
            RETURN_META_VALUE(MRES_SUPERCEDE, pokebot::game::game.GetBotArgCount());
        }
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };

    meta_engfuncs.pfnSetClientMaxspeed = [](const edict_t* pEdict, float fNewMaxspeed) {
        RETURN_META(MRES_IGNORED);
    };

    meta_engfuncs.pfnGetPlayerUserId = [](edict_t* e) -> int {
        RETURN_META_VALUE(MRES_IGNORED, 0);
    };

    memcpy(pengfuncsFromEngine, &meta_engfuncs, sizeof(enginefuncs_t));
    return TRUE;
}