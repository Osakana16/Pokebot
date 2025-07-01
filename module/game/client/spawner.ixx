export module pokebot.game.client: spawner;

import pokebot.util;

export namespace pokebot::game::client {
	class EntitySpawner {
	public:
		virtual EntitySpawner& ApplyName(const util::PlayerName&) = 0;
		virtual bool Spawn() noexcept = 0;
	};

	class FakeClientSpawner : public EntitySpawner {
	public:
		FakeClientSpawner& ApplyName(const util::PlayerName& player_name) final {
			
			return *this;
		}

		bool Spawn() noexcept final {
		 	return true;
		}
	};
}