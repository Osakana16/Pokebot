export module pokebot.plugin.event.client_connection;
import pokebot.plugin.event;

import pokebot.common.event_handler;

export namespace pokebot::plugin::event {
	class ClientConnectionObservable final : public ClientInformationObservable {
		std::forward_list<std::shared_ptr<common::Observer<ClientInformation>>> observers{};
	public:
		~ClientConnectionObservable() final {}

		ClientConnectionObservable() {}

		void AddObserver(std::shared_ptr<common::Observer<ClientInformation>> observer) final {
			observers.push_front(observer);
		}

		void Notifyobservers(const ClientInformation& event) final {
			for (auto& observer : observers)
				observer->OnEvent(event);
		}
	};
}