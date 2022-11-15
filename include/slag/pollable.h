#pragma once

#include <initializer_list>
#include <bitset>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_list.h"

#define SLAG_POLLABLE_EVENTS(X) \
    X(READABLE)                 \
    X(WRITABLE)                 \
    X(CLOSED)

namespace slag {

    class Pollable {
    public:
        enum class Event {
    #define X(SLAG_POLLABLE_EVENT) SLAG_POLLABLE_EVENT,
            SLAG_POLLABLE_EVENTS(X)
    #undef X
        };

        static constexpr size_t EVENT_COUNT = 0
    #define X(SLAG_POLLABLE_EVENT) + 1
            SLAG_POLLABLE_EVENTS(X)
    #undef X
        ;

        using EventMask = std::bitset<EVENT_COUNT>;

    public:
        class Observer {
            friend class Pollable;

        public:
            virtual ~Observer() = default;

            virtual void handle_pollable_event(Pollable& pollable, Event event) = 0;
            virtual void handle_pollable_destroyed(Pollable& pollable) = 0;

        private:
            IntrusiveListNode hook_;
        };

    public:
        Pollable() = default;
        Pollable(Pollable&&) = default;
        Pollable(const Pollable&) = delete;
        ~Pollable();

        Pollable& operator=(Pollable&&) = default;
        Pollable& operator=(const Pollable&) = delete;

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

    [[nodiscard]] size_t to_index(Pollable::Event event);
    [[nodiscard]] const char* to_string(Pollable::Event event);
    [[nodiscard]] Pollable::EventMask make_pollable_events(std::initializer_list<Pollable::Event> events);

}
