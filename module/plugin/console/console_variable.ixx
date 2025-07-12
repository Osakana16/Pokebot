export module pokebot.plugin.console.variable;

import pokebot.util;

export namespace pokebot::plugin::console {
	// variable type
	enum class Var {
		Normal = 0,
		ReadOnly,
		Password,
		NoServer,
		GameRef
	};

	// ConVar class from YapBot © Copyright YaPB Project Developers
	// 
	// simplify access for console variables
	class ConVar final {
	public:
		cvar_t* ptr;

		ConVar() = delete;
		~ConVar() = default;


		ConVar(const char* name, const char* initval, Var type = Var::NoServer, bool regMissing = false, const char* regVal = nullptr);
		ConVar(const char* name, const char* initval, const char* info, bool bounded = true, float min = 0.0f, float max = 1.0f, Var type = Var::NoServer, bool regMissing = false, const char* regVal = nullptr);

		explicit operator bool() const POKEBOT_NOEXCEPT { return ptr->value > 0.0f; }
		explicit operator int() const POKEBOT_NOEXCEPT { return static_cast<int>(ptr->value); }
		explicit operator float() const POKEBOT_NOEXCEPT { return ptr->value; }
		explicit operator const char* () const POKEBOT_NOEXCEPT { return ptr->string; }

		void operator=(const float val) POKEBOT_NOEXCEPT { g_engfuncs.pfnCVarSetFloat(ptr->name, val); }
		void operator=(const int val) POKEBOT_NOEXCEPT { operator=(static_cast<float>(val)); }
		void operator=(const char* val) POKEBOT_NOEXCEPT { g_engfuncs.pfnCvar_DirectSet(ptr, const_cast<char*>(val)); }

	};

	struct ConVarReg {
		cvar_t reg;
		util::fixed_string<64u> info;
		util::fixed_string<64u> init;
		const char* regval;
		class ConVar* self;
		float initial, min, max;
		bool missing;
		bool bounded;
		Var type;
	};
	
	extern ConVar poke_freeze;
	extern ConVar poke_fight;
	extern ConVar poke_buy;
}