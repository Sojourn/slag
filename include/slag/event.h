#pragma once

#include <vector>

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

        virtual void handle_event_set(Event& event, void* user_data) = 0;
        virtual void handle_event_destroyed(void* user_data) = 0;

    private:
        using EventWait = std::pair<Event*, void*>;
        using EventWaits = std::vector<EventWait>;

        [[nodiscard]] EventWaits::iterator find_wait(Event& event);

    private:
         EventWaits event_waits_;
    };

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
        // TODO: encode this as a tagged pointer instead
        EventObserver* observer_;
        bool           is_set_;
    };

}
