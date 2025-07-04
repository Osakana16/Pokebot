export module pokebot.plugin.event.map_loaded;
import pokebot.common.event_handler;

export namespace pokebot::plugin::event {
	class MapLoadedObservable final : public common::Observable<int> {
		std::forward_list<std::shared_ptr<common::Observer<int>>> observers{};
	public:
		~MapLoadedObservable() final {}

		MapLoadedObservable() {}

		void AddObserver(std::shared_ptr<common::Observer<int>> observer) final {
			observers.push_front(observer);
		}

		void Notifyobservers(const int& event) final {
			for (auto& observer : observers)
				observer->OnEvent(event);
		}
	};
}