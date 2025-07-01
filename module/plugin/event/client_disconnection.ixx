export module pokebot.plugin.event: client_disconnection;

import pokebot.common.event_handler;

export namespace pokebot {
	struct ClientInformation final {
		edict_t* entity;
		const char* Name;
		const char* Address;
	};

	class ClientDisconnectionObservable final : public common::Observable<int> {
		std::forward_list<std::shared_ptr<common::Observer<int>>> observers{};
	public:
		~ClientDisconnectionObservable() final {}

		ClientDisconnectionObservable() {

		}

		void AddObserver(std::shared_ptr<common::Observer<int>> observer) final {
			observers.push_front(observer);
		}

		void Notifyobservers(const int& event) final {
			for (auto& observer : observers)
				observer->OnEvent(event);
		}
	};
}