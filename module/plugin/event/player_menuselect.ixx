export module pokebot.plugin.event: player_menuselect;
import pokebot.common.event_handler;

export namespace pokebot::plugin::event {
	class MapLoadedObservable final : public common::Observable<edict_t*> {
		std::forward_list<std::shared_ptr<common::Observer<edict_t*>>> observers{};
	public:
		~MapLoadedObservable() final {}

		MapLoadedObservable() {}

		void AddObserver(std::shared_ptr<common::Observer<edict_t*>> observer) final {
			observers.push_front(observer);
		}

		void Notifyobservers(const edict_t* const& event) final {
			for (auto& observer : observers)
				observer->OnEvent(event);
		}
	};
}