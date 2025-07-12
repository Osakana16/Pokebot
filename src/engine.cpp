module;
#include "common.hpp"

module pokebot: plugin;
import pokebot.engine;
import pokebot.bot;
import pokebot.game;
import pokebot.game.player;
import pokebot.game.util;
import pokebot.util;
import pokebot.util.tracer;
import pokebot.terrain.graph;

namespace {
    int current_message{};
    
    using TextCache = pokebot::util::fixed_string<120u>;
    std::vector<std::variant<int, float, TextCache>> args{};
}

const edict_t* engine_target_edict = nullptr;
bool is_bot{};
bool is_host{};

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
    meta_engfuncs.pfnMessageBegin = pokebot::engine::EngineInterface::OnMessageBegin;
    meta_engfuncs.pfnMessageEnd = pokebot::engine::EngineInterface::OnMessageEnd;
    meta_engfuncs.pfnWriteByte = pokebot::engine::EngineInterface::OnWriteByte;
    meta_engfuncs.pfnWriteChar = pokebot::engine::EngineInterface::OnWriteChar;
    meta_engfuncs.pfnWriteShort = pokebot::engine::EngineInterface::OnWriteShort;
    meta_engfuncs.pfnWriteLong = pokebot::engine::EngineInterface::OnWriteLong;
    meta_engfuncs.pfnWriteAngle = pokebot::engine::EngineInterface::OnWriteAngle;
    meta_engfuncs.pfnWriteCoord = pokebot::engine::EngineInterface::OnWriteCoord;
    meta_engfuncs.pfnWriteString = pokebot::engine::EngineInterface::OnWriteString;
    meta_engfuncs.pfnWriteEntity = pokebot::engine::EngineInterface::OnWriteEntity;
    meta_engfuncs.pfnClientCommand = pokebot::engine::EngineInterface::OnClientCommand;
    meta_engfuncs.pfnCmd_Args = pokebot::engine::EngineInterface::OnArgs;
    meta_engfuncs.pfnCmd_Argv = pokebot::engine::EngineInterface::OnArgv;
    meta_engfuncs.pfnCmd_Argc = pokebot::engine::EngineInterface::OnArgc;
    memcpy(pengfuncsFromEngine, &meta_engfuncs, sizeof(enginefuncs_t));
    return TRUE;
}