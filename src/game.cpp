import std;
import pokebot.game;
import pokebot.game.util;
import pokebot.game.client;
import pokebot.util;
import pokebot.util.tracer;
import pokebot.common.event_handler;
import pokebot.plugin.event;

namespace pokebot {
	namespace game {
		Game::Game(common::Observable<void>* frame_update_observable,
				   plugin::event::ClientInformationObservable* client_connection_observable,
				   plugin::event::ClientInformationObservable* client_disconnection_observable) 
		{
			class GameUpdateObserver : public common::Observer<void> {
			public:
				~GameUpdateObserver() final {}

				void OnEvent() final {}
			};

			class ClientConnectionObserver : public common::Observer<plugin::event::ClientInformation> {
			public:
				~ClientConnectionObserver() final {}

				void OnEvent(const plugin::event::ClientInformation&) final {}
			};

			class ClientDisconnectionObserver : public common::Observer<plugin::event::ClientInformation> {
			public:
				~ClientDisconnectionObserver() final {}
				
				void OnEvent(const plugin::event::ClientInformation&) final {}
			};

			frame_update_observable->AddObserver(std::make_shared<GameUpdateObserver>());
			client_connection_observable->AddObserver(std::make_shared<ClientConnectionObserver>());
			client_disconnection_observable->AddObserver(std::make_shared<ClientDisconnectionObserver>());
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