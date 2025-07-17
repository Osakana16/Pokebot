export module pokebot.common.event_handler;

export namespace pokebot::common {
    // - Observer -

    template <typename Event>
    class Observer {
    public:
        virtual ~Observer() = 0 {}
        virtual void OnEvent(const Event& event) = 0;
    };


    template <typename Event>
    class Observer<Event*> {
    public:
        virtual ~Observer() = 0 {}
        virtual void OnEvent(const Event* const& event) = 0;
    };

    template<>
    class Observer<void> {
    public:
        virtual ~Observer() = 0 {}
        virtual void OnEvent() = 0;
    };

    template <typename Event>
    class NormalObserver : public Observer<Event> {
        std::function<void(const Event&)> callback;
    public:
        NormalObserver(decltype(callback) callback_) : callback(callback_) {}

        ~NormalObserver() final {}

        void OnEvent(const Event& event) final {
            callback(event);
        }
    };


    template <typename Event>
    class NormalObserver<Event*> : public Observer<Event> {
        std::function<void(const Event* const&)> callback;
    public:
        NormalObserver(decltype(callback) callback_) : callback(callback_) {}

        ~NormalObserver() final {}

        void OnEvent(const Event* const& event) final {
            callback(event);
        }
    };

    template<>
    class NormalObserver<void> : public Observer<void> {
        std::function<void()> callback;
    public:
        NormalObserver(decltype(callback) callback_) : callback(callback_) {}

        ~NormalObserver() final {}

        void OnEvent() final {
            callback();
        }
    };

    // - Observable -

    template <typename Event>
    class Observable {
    public:
        virtual ~Observable() = 0 {}
        virtual void AddObserver(std::shared_ptr<Observer<Event>> observer) = 0;
        virtual void NotifyObservers(const Event& event) = 0;
    };

    template <typename Event>
    class Observable<Event*> {
    public:
        virtual ~Observable() = 0 {}
        virtual void AddObserver(std::shared_ptr<Observer<Event*>> observer) = 0;
        virtual void NotifyObservers(const Event* const& event) = 0;
    };

    template<>
    class Observable<void> {
    public:
        virtual ~Observable() = 0 {}
        virtual void AddObserver(std::shared_ptr<Observer<void>> observer) = 0;
        virtual void NotifyObservers() = 0;
    };

    template<typename Event>
    class NormalObservable : public Observable<Event> {
        std::forward_list<std::shared_ptr<common::Observer<Event>>> observers{};
    public:
        ~NormalObservable() final {}

        NormalObservable() {}

        void AddObserver(std::shared_ptr<common::Observer<Event>> observer) final {
            observers.push_front(observer);
        }

        void NotifyObservers(const Event& event) final {
            for (auto& observer : observers)
                observer->OnEvent(event);
        }
    };

    template<typename Event>
    class NormalObservable<Event*> : public Observable<Event*> {
        std::forward_list<std::shared_ptr<common::Observer<Event*>>> observers{};
    public:
        ~NormalObservable() final {}

        NormalObservable() {}

        void AddObserver(std::shared_ptr<common::Observer<Event*>> observer) final {
            observers.push_front(observer);
        }

        void NotifyObservers(const Event* const& event) final {
            for (auto& observer : observers)
                observer->OnEvent(event);
        }
    };

    template<>
    class NormalObservable<void> : public Observable<void> {
        std::forward_list<std::shared_ptr<common::Observer<void>>> observers{};
    public:
        ~NormalObservable() final {}

        NormalObservable() {}

        void AddObserver(std::shared_ptr<common::Observer<void>> observer) final {
            observers.push_front(observer);
        }

        void NotifyObservers() final {
            for (auto& observer : observers)
                observer->OnEvent();
        }
    };

    template<typename Event, typename Key>
    class RemovableObservable : public Observable<Event> {
    public:
        virtual ~RemovableObservable() = 0 {}
        virtual void AddObserver(const Key, std::shared_ptr<Observer<Event>> observer) = 0;
        virtual void Remove(const Key) = 0;
    };

    template<typename Event, typename DynamicArray>
    class SequenceObservable : public RemovableObservable<Event, std::uint64_t> {
        DynamicArray observers;
    public:
        ~SequenceObservable() override {}

        void AddObserver(const std::uint64_t index, std::shared_ptr<Observer<Event>> observer) override {
            observers.insert(observers.begin() + index, observer);
        }

        void NotifyObservers() final {
            for (auto& observer : observers)
                observer->OnEvent();
        }

        void Remove(const std::uint64_t index) override {
            observers.erase(observers.begin() + index);
        }
    };

    template<typename Event, typename Key, typename Hash>
    class MapObservable : public RemovableObservable<Event, Key> {
        std::unordered_map<Key, std::shared_ptr<Observer<Event>>, Hash> observers;
    public:
        ~MapObservable() override {}

        void AddObserver(std::shared_ptr<Observer<Event>> observer) override {}
        void NotifyObservers() override {}

        void AddObserver(const Key key, std::shared_ptr<Observer<Event>> observer) override {
            observers.insert({ key, observer });
        }

        void NotifyObservers(const Key key) {
            observers.at(key)->OnEvent();
        }

        void Remove(const Key key) override {
            observers.erase(key);
        }
    };
}