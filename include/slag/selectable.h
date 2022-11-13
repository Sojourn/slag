#pragma once

#include <initializer_list>
#include <bitset>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_list.h"

#define SLAG_SELECTABLE_EVENTS(X) \
    X(READABLE)                 \
    X(WRITABLE)                 \
    X(CLOSED)

namespace slag {

    // Rename this to Awaitable? Pollable?
    class Selectable {
    public:
        enum class Event {
    #define X(SLAG_SELECTOR_EVENT) SLAG_SELECTOR_EVENT,
            SLAG_SELECTABLE_EVENTS(X)
    #undef X
        };

        static constexpr size_t EVENT_COUNT = 0
    #define X(SLAG_SELECTOR_EVENT) + 1
            SLAG_SELECTABLE_EVENTS(X)
    #undef X
        ;

        using EventMask = std::bitset<EVENT_COUNT>;

    public:
        class Observer {
            friend class Selectable;

        public:
            virtual ~Observer() = default;

            virtual void handle_selectable_event(Selectable& selectable, Event event) = 0;
            virtual void handle_selectable_destroyed(Selectable& selectable) = 0;

        private:
            IntrusiveListNode hook_;
        };

    public:
        Selectable() = default;
        Selectable(Selectable&&) = default;
        Selectable(const Selectable&) = delete;
        ~Selectable();

        Selectable& operator=(Selectable&&) = default;
        Selectable& operator=(const Selectable&) = delete;

        [[nodiscard]] const EventMask& events() const;
        void add_observer(Observer& observer);
        void remove_observer(Observer& observer);

    protected:
        void set_event(Event event, bool value);

    private:
        using ObserverList = IntrusiveList<Observer, &Observer::hook_>;

        EventMask    events_;
        ObserverList observers_;
    };

    [[nodiscard]] size_t to_index(Selectable::Event event);
    [[nodiscard]] const char* to_string(Selectable::Event event);
    [[nodiscard]] Selectable::EventMask make_selectable_events(std::initializer_list<Selectable::Event> events);

}
