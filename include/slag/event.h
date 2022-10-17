#pragma once

#include <vector>
#include "slag/tagged_pointer.h"

namespace slag {

    class Event;

    class EventObserver {
        friend class Event;

    public:
        EventObserver() = default;
        EventObserver(EventObserver&&) noexcept = delete;
        EventObserver(const EventObserver&) = delete;
        virtual ~EventObserver();

        EventObserver& operator=(EventObserver&&) noexcept = delete;
        EventObserver& operator=(const EventObserver&) = delete;

        void wait(Event& event, void* user_data);
        void cancel_wait(Event& event);

        // TODO: remove the event parameter
        virtual void handle_event_set(Event& event, void* user_data) = 0;
        virtual void handle_event_destroyed(void* user_data) = 0;

    private:
        using EventWait  = std::pair<Event*, void*>;
        using EventWaits = std::vector<EventWait>;

        [[nodiscard]] EventWaits::iterator find_wait(Event& event);

    private:
         EventWaits event_waits_; // intrusive list?
    };

    // TODO: Think about making an EventBase which can be waited on,
    //       but doesn't expose Event::set/reset.
    //
    class Event {
        friend class EventObserver;

    public:
        Event();
        Event(Event&&) noexcept = delete;
        Event(const Event&) = delete;
        ~Event();

        Event& operator=(Event&&) noexcept = delete;
        Event& operator=(const Event&) = delete;

        [[nodiscard]] bool is_set() const;
        void set();
        void reset();

    private:
        tagged_pointer<EventObserver, 1> tagged_observer_pointer_;
    };

}
