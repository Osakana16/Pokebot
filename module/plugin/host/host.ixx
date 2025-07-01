export module pokebot.plugin.host;

namespace pokebot::plugin {
	class Host {
		edict_t* host{};


	public:
		void ShowMenu();

		const edict_t* AsEdict() const POKEBOT_NOEXCEPT { return host; }
		bool IsHostValid() const POKEBOT_NOEXCEPT;
		void SetHost(edict_t* const target) POKEBOT_NOEXCEPT;
		const char* const HostName() const POKEBOT_NOEXCEPT;
		const Vector& Origin() const POKEBOT_NOEXCEPT;
		void Update();
	};
}