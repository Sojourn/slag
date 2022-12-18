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

    enum class PollableEvent {
#define X(SLAG_POLLABLE_EVENT) SLAG_POLLABLE_EVENT,
        SLAG_POLLABLE_EVENTS(X)
#undef X
    };

    [[nodiscard]] size_t to_index(PollableEvent event);
    [[nodiscard]] const char* to_string(PollableEvent event);

    static constexpr size_t POLLABLE_EVENT_COUNT = 0
#define X(SLAG_POLLABLE_EVENT) + 1
        SLAG_POLLABLE_EVENTS(X)
#undef X
    ;

    struct PollableEventMask : std::bitset<POLLABLE_EVENT_COUNT> {
        using Base = std::bitset<POLLABLE_EVENT_COUNT>;

        using Base::Base;
        using Base::set;
        using Base::test;
        using Base::reset;

        PollableEventMask(PollableEvent event);
        PollableEventMask(std::initializer_list<PollableEvent> events);

        [[nodiscard]] bool test(PollableEvent event) const;
        [[nodiscard]] PollableEventMask& set(PollableEvent event, bool value = true);
    };

    [[nodiscard]] std::string to_string(PollableEventMask events);

    class Pollable {
    public:
        using Event = PollableEvent;
        using EventMask = PollableEventMask;

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
        void set_event(Event event, bool value = true);

    private:
        using ObserverList = IntrusiveList<Observer, &Observer::hook_>;

        EventMask    events_;
        ObserverList observers_;
    };

}
