module;
#include "goldsrc.hpp"
#include <ranges>
export module pokebot.game.scenario: hostage_rescue_maanger;
import pokebot.game.scenario.manager;
import pokebot.util;
import pokebot.game.util;

namespace pokebot::game::scenario {
	class Hostage final {
		Hostage() = default;
		Hostage(const Hostage&) = default;
		Hostage& operator=(const Hostage&) = delete;

		util::Time time{};

		const edict_t* entity;
		pokebot::util::PlayerName owner_name{};
	public:
		operator const edict_t* const () const POKEBOT_NOEXCEPT {
			return entity;
		}

		static Hostage AttachHostage(const edict_t* Hostage_Entity) POKEBOT_NOEXCEPT {
			assert(Hostage_Entity != nullptr);
			Hostage hostage{};
			hostage.entity = Hostage_Entity;
			hostage.owner_name.clear();
			return hostage;
		}

		bool RecoginzeOwner(const std::string_view& Client_Name) POKEBOT_NOEXCEPT;

		void Update() POKEBOT_NOEXCEPT;
		bool IsUsed() const POKEBOT_NOEXCEPT { return false; }
		bool IsOwnedBy(const std::string_view& Name) const POKEBOT_NOEXCEPT { return (IsUsed() && owner_name.data() == Name); }
		bool IsReleased() const POKEBOT_NOEXCEPT { return (entity->v.effects & EF_NODRAW); }
		const Vector& Origin() const POKEBOT_NOEXCEPT {
			return entity->v.origin;
		}
#if 0
		Hostage(Game *game_, Hostage&& h) : game(game_) {
			owner_name = std::move(h.owner_name);
			assert(h.owner_name.empty());
			entity = h.entity;
			h.entity = nullptr;
		}

		bool RecoginzeOwner(const std::string_view& Client_Name) POKEBOT_NOEXCEPT {
			auto client = game->clients.Get(Client_Name.data());
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

			auto owner = game->clients.Get(owner_name.data());
			const bool Is_Owner_Terrorist = owner->GetTeam() == game::Team::T;
			if (IsReleased() || owner->GetTeam() == game::Team::T || game::Distance(owner->origin, entity->v.origin) > 200.0f)
				owner_name.clear();
		}

		bool IsUsed() const POKEBOT_NOEXCEPT { return game->PlayerExists(owner_name.data()); }
#endif
	};

	class HostageRescueManager : public ScenarioManager {
		std::vector<Hostage> hostages{};

		size_t GetNumberOfHostages() const noexcept {
			return hostages.size();
		}

		size_t GetNumberOfLivingHostages() const noexcept {
			auto&& living_hostages = (hostages | std::views::filter([](const Hostage& Target) -> bool { return static_cast<const edict_t*>(Target)->v.health > 0; }));
			return std::distance(living_hostages.begin(), living_hostages.end());
		}

		size_t GetNumberOfRescuedHostages() const noexcept {
			auto&& living_hostages = (hostages | std::views::filter([](const Hostage& Target) -> bool { return bool(static_cast<const edict_t*>(Target)->v.effects & EF_NODRAW); }));
			return std::distance(living_hostages.begin(), living_hostages.end());
		}

		std::optional<Vector> GetHostageOrigin(const int Index) const noexcept {
			if (Index < 0 || Index >= hostages.size())
				return std::nullopt;

			return hostages[Index].Origin();
		}

		bool IsHostageUsed(const int Index) const POKEBOT_NOEXCEPT {
			return hostages[Index].IsUsed();
		}

		bool IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name) {
			return hostages[Index].IsOwnedBy(Owner_Name);
		}

		const edict_t* const GetClosedHostage(const Vector& Origin, const float Base_Distance) {
			for (auto& hostage : hostages) {
				if (Distance(hostage.Origin(), Origin) <= Base_Distance) {
					return hostage;
				}
			}
			return nullptr;
		}
	public:
		void Update() final {}
	};
}