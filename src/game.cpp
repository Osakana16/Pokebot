import std;
import pokebot.game;
import pokebot.game.util;
import pokebot.game.client;
import pokebot.util;
import pokebot.util.tracer;

namespace pokebot {
	namespace game {
		ConVar::ConVar(const char* name, const char* initval, Var type, bool regMissing, const char* regVal) {
			game.AddCvar(name, initval, "", false, 0.0f, 0.0f, type, regMissing, regVal, this);
		}
		
		ConVar::ConVar(const char* name, const char* initval, const char* info, bool bounded, float min, float max, Var type, bool regMissing, const char* regVal) {
			game.AddCvar(name, initval, info, bounded, min, max, type, regMissing, regVal, this);
		}

		ConVar poke_freeze{ "pk_freeze", "0" };
		ConVar poke_fight{ "pk_fight", "1"};
		ConVar poke_buy{ "pk_buy", "1"};

		Hostage Hostage::AttachHostage(const edict_t* Hostage_Entity) POKEBOT_NOEXCEPT {
			assert(Hostage_Entity != nullptr);
			Hostage hostage{};
			hostage.entity = Hostage_Entity;
			hostage.owner_name.clear();
			return hostage;
		}

		bool Hostage::RecoginzeOwner(const std::string_view& Client_Name) POKEBOT_NOEXCEPT {
			auto client = game.clients.Get(Client_Name.data());
			if (client != nullptr && game::Distance(client->origin, entity->v.origin) < 83.0f && client->GetTeam() == game::Team::CT) {
				if (owner_name.c_str() == Client_Name) {
					owner_name.clear();
				} else {
					owner_name = Client_Name.data();
				}
				return true;
			}
			return false;
		}
		

		void Hostage::Update() POKEBOT_NOEXCEPT {
			if (owner_name.empty())
				return;
			
			auto owner = game.clients.Get(owner_name.data());
			const bool Is_Owner_Terrorist = owner->GetTeam() == game::Team::T;
			if (IsReleased() || owner->GetTeam() == game::Team::T || game::Distance(owner->origin, entity->v.origin) > 200.0f)
				owner_name.clear();
		}

		bool Hostage::IsUsed() const POKEBOT_NOEXCEPT { return game.PlayerExists(owner_name.data()); }
		bool Hostage::IsOwnedBy(const std::string_view& Name) const POKEBOT_NOEXCEPT { return (IsUsed() && owner_name.data() == Name); }
	 	bool Hostage::IsReleased() const POKEBOT_NOEXCEPT { return (entity->v.effects & EF_NODRAW); }
		const Vector& Hostage::Origin() const POKEBOT_NOEXCEPT {
			return entity->v.origin;
		}




		bool Host::IsHostValid() const POKEBOT_NOEXCEPT {
			return host != nullptr;
		}

		const char* const Host::HostName() const POKEBOT_NOEXCEPT { return STRING(host->v.netname); }
		const Vector& Host::Origin() const POKEBOT_NOEXCEPT { return host->v.origin; }

		void Host::SetHost(edict_t* const target) POKEBOT_NOEXCEPT {
			host = target;
		}
		
		void Host::Update() {
			if (host != nullptr) {
#ifndef NDEBUG
				host->v.health = 255;
#endif
				if (game::is_enabled_auto_waypoint &&  (host->v.deadflag != DEAD_DEAD && host->v.deadflag != DEAD_DYING && host->v.movetype != MOVETYPE_NOCLIP)) {
#if !USE_NAVMESH
					pokebot::node::world.Add(pokebot::game::game.host.Origin(), pokebot::node::GoalKind::None);
#endif
				}
			}
		}
	}

	namespace entity {
		bool InViewCone(const edict_t* const self, const Vector& Origin) POKEBOT_NOEXCEPT {
			MAKE_VECTORS(self->v.angles);
			const auto Vector_2D_Los = (Origin - self->v.origin).Make2D().Normalize();
			const auto Dot = DotProduct(Vector_2D_Los, gpGlobals->v_forward.Make2D());
			return (Dot > 0.50);
		}

		bool IsVisible(const edict_t* const self, const Vector& Origin) POKEBOT_NOEXCEPT {
			// look through caller's eyes
			TraceResult tr;
			UTIL_TraceLine(self->v.origin + self->v.view_ofs, Origin, dont_ignore_monsters, ignore_glass, const_cast<edict_t*>(self), &tr);
			return (tr.flFraction >= 1.0);	// line of sight is not established or valid
		}

		bool CanSeeEntity(const edict_t* const self, const const edict_t* Target) POKEBOT_NOEXCEPT {
			const auto Body = Target->v.origin;
			const auto Head = Target->v.origin + Target->v.view_ofs;
			
			const bool Is_Body_In_ViewCone = InViewCone(self, Body);
			const bool Is_Body_Visible = IsVisible(self, Body);

			const bool Is_Head_In_ViewCone = InViewCone(self, Head);
			const bool Is_Head_Visible = IsVisible(self, Head);


			const bool Is_Body_In_FOV = Is_Body_Visible && Is_Body_In_ViewCone;
			const bool Is_Head_In_FOV = Is_Head_In_ViewCone && Is_Head_Visible;
			return (Is_Body_In_FOV || Is_Head_In_FOV);
		}
	}
}