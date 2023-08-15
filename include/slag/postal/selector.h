#pragma once

#include <span>
#include <cstddef>
#include "slag/intrusive_queue.h"
#include "slag/postal/event.h"

namespace slag::postal {

    class Selector {
    public:
        void insert(Event& event);
        void insert(std::span<Event*> events);

        void remove(Event& event);
        void remove(std::span<Event*> events);

        Event* select();
        size_t select(std::span<Event*> events);

    private:
        friend class Event;

        void handle_readiness_change(Event& event);
        void update_readiness();

    private:
        using ReadyQueue = IntrusiveQueue<Event, &Event::selector_hook_>;

        Event      event_;
        ReadyQueue queue_;
    };

}
