#pragma once
namespace pokebot::config {
	[[nodiscard]] cvar_t* CreateCVar(const char* const Name);

	inline cvar_t* waypoint_auto_save{};
	inline cvar_t* waypoint_auto_generate{};
	inline cvar_t* freeze{};
	inline cvar_t* combat{};
	inline cvar_t* camp{};

	inline cvar_t* defuse{};
	inline cvar_t* plant{};
	inline cvar_t* rescue{};
	inline cvar_t* vip_escape{};
	inline cvar_t* escape{};

	inline cvar_t* pistol{};
	inline cvar_t* shotgun{};
	inline cvar_t* submachinegun{};
	inline cvar_t* rifle{};
	inline cvar_t* sniper{};
	inline cvar_t* machinegun{};
	inline cvar_t* grenade{};
	inline cvar_t* shield{};

	inline cvar_t* usp{};
	inline cvar_t* glock{};
	inline cvar_t* p228{};
	inline cvar_t* deagle{};
	inline cvar_t* five_seven{};
	inline cvar_t* elite{};
	inline cvar_t* m3{};
	inline cvar_t* xm1014{};
	inline cvar_t* tmp{};
	inline cvar_t* mac10{};
	inline cvar_t* mp5{};
	inline cvar_t* ump45{};
	inline cvar_t* p90{};
	inline cvar_t* galil{};
	inline cvar_t* famas{};
	inline cvar_t* m4a1{};
	inline cvar_t* ak47{};
	inline cvar_t* aug{};
	inline cvar_t* sg552{};
	inline cvar_t* scout{};
	inline cvar_t* sg550{};
	inline cvar_t* g3sg1{};
	inline cvar_t* awp{};
	inline cvar_t* m249{};
	inline cvar_t* knife{};
}