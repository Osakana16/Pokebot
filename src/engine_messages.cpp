module;
#include "goldsrc.hpp"
module pokebot.engine;

namespace pokebot::engine {
	void EngineInterface::OnMessageBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* edict) {
		if (gpGlobals->deathmatch) {
			engine_target_edict = edict;
			current_message = msg_type;
		}
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnMessageEnd() {
		if (gpGlobals->deathmatch) {
			static std::unordered_map<int, void(*)()> messages
			{
				{ GET_USER_MSG_ID(PLID, "VGUIMenu", nullptr), OnVGUIMenuShown },
				{ GET_USER_MSG_ID(PLID, "ShowMenu", nullptr), OnShowMenu },
				{ GET_USER_MSG_ID(PLID, "WeaponList", nullptr), OnWeaponListCalled },
				{ GET_USER_MSG_ID(PLID, "ScoreAttrib", nullptr), OnVGUIMenuShown },
				{ GET_USER_MSG_ID(PLID, "CurWeapon", nullptr), OnVGUIMenuShown },
				{ GET_USER_MSG_ID(PLID, "AmmoX", nullptr), OnPlayerAmmoPickup },
				{ GET_USER_MSG_ID(PLID, "AmmoPickup", nullptr), OnPlayerAmmoPickup },
				{ GET_USER_MSG_ID(PLID, "Money", nullptr), OnMoneyChanged },
				{ GET_USER_MSG_ID(PLID, "Damage", nullptr), OnPlayerDamageTaken },
				{ GET_USER_MSG_ID(PLID, "TextMsg", nullptr), OnTextMessageSent },
				{ GET_USER_MSG_ID(PLID, "StatusIcon", nullptr), OnStatusIconShown },
				{ GET_USER_MSG_ID(PLID, "HLTV", nullptr), OnHLTVStart },
			};

			if (auto it = messages.find(current_message); it != messages.end()) {
				it->second();
			}
			args.clear();
		}
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteByte(int value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteChar(int value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteShort(int value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteLong(int value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteAngle(float value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteCoord(float value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteString(const char* value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnWriteEntity(int value) {
		PushArg(value);
		RETURN_META(MRES_IGNORED);
	}

	void EngineInterface::OnClientCommand(edict_t* pEdict, const char* szFmt, ...) {
		if (gpGlobals->deathmatch) {
			if (pEdict->v.flags & (FL_FAKECLIENT | pokebot::util::Third_Party_Bot_Flag))
				RETURN_META(MRES_SUPERCEDE);
		}
		RETURN_META(MRES_IGNORED);
	}

	const char* EngineInterface::OnArgs() {
		if (is_bot_command) {
			RETURN_META_VALUE(MRES_SUPERCEDE, bot_args.at(0).c_str()); // returns the wanted argument
		}
		RETURN_META_VALUE(MRES_IGNORED, nullptr);
	}

	const char* EngineInterface::OnArgv(int argc) {
		if (is_bot_command) {
			RETURN_META_VALUE(MRES_SUPERCEDE, bot_args.at(bot_args.size() == argc ? argc - 1 : argc).c_str()); // returns the wanted argument
		}
		RETURN_META_VALUE(MRES_IGNORED, nullptr);
	}

	int EngineInterface::OnArgc() {
		RETURN_META_VALUE(MRES_IGNORED, is_bot_command ? bot_args.size() : 0);
	}

	void EngineInterface::PushArg(int value) {
		if (gpGlobals->deathmatch) {
			args.push_back(value);
		}
	}

	void EngineInterface::PushArg(float value) {
		if (gpGlobals->deathmatch) {
			args.push_back(value);
		}
	}

	void EngineInterface::PushArg(const char* const value) {
		if (gpGlobals->deathmatch) {
			args.push_back(value);
		}
	}
}