module pokebot.game.client: client;
import pokebot.game;

namespace pokebot::game::client {
	bool Client::HasHostages() const noexcept {
		for (int i = 0; i < game::game.GetNumberOfHostages(); i++) {
			if (game::game.IsHostageOwnedBy(i, Name())) {
				return true;
			}
		}
		return false;
	}


	bool Client::CanSeeFriend() const noexcept {
		for (auto& other : game.clients.GetAll()) {
			if (entity::CanSeeEntity(*this, other.second) && other.second.GetTeam() == GetTeam()) {
				return true;
			}
		}
		return false;
	}


	void Client::GetEnemyNamesWithinView(pokebot::util::PlayerName player_names[32]) const noexcept {
		int i = 0;
		for (const auto& other : game.clients.GetAll()) {
			if (other.second.IsDead() || other.second.GetTeam() == GetTeam()) {
				continue;
			}

			if (entity::CanSeeEntity(*this, other.second)) {
				player_names[i++] = other.first.data();
			}
		}
	}

	void Client::GetEntityNamesInView(pokebot::util::PlayerName player_names[32]) const noexcept {
		int i = 0;
		for (auto& other : game.clients.GetAll()) {
			if (entity::CanSeeEntity(*this, other.second)) {
				player_names[i++] = other.first.data();
			}
		}
	}
}