export module pokebot.common.event_handler;

export namespace pokebot::common {
    template <typename Event>
    class Observer {
    public:
        virtual ~Observer() = 0 {}
        virtual void OnEvent(const Event& event) = 0;
    };
    
    template <typename Event>
    class Observable {
    public:
        virtual ~Observable() = 0 {}
        virtual void AddObserver(std::shared_ptr<Observer<Event>> observer) = 0;
        virtual void Notifyobservers(const Event& event) = 0;
    };
    
    template <typename Event>
    class Observer<Event*> {
    public:
        virtual ~Observer() = 0 {}
        virtual void OnEvent(const Event* const& event) = 0;
    };
    
    template <typename Event>
    class Observable<Event*> {
    public:
        virtual ~Observable() = 0 {}
        virtual void AddObserver(std::shared_ptr<Observer<Event*>> observer) = 0;
        virtual void Notifyobservers(const Event* const& event) = 0;
    };

    template<>
    class Observer<void> {
    public:
        virtual ~Observer() = 0 {}
        virtual void OnEvent() = 0;
    };


    template<>
    class Observable<void> {
    public:
        virtual ~Observable() = 0 {}
        virtual void AddObserver(std::shared_ptr<Observer<void>> observer) = 0;
        virtual void Notifyobservers() = 0;
    };
}