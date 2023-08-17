#pragma once

#include <span>
#include <cstddef>
#include "slag/intrusive_queue.h"
#include "slag/postal/event.h"
#include "slag/postal/pollable.h"

namespace slag::postal {

    class Selector : public Pollable<PollableType::READABLE> {
    public:
        Event& readable_event() override;

        void insert(Event& event);
        void insert(std::span<Event*> events); // Can be sparse.

        void remove(Event& event);
        void remove(std::span<Event*> events); // Can be sparse.

        Event* select();
        size_t select(std::span<Event*> events);

    private:
        friend class Event;

        void handle_readiness_change(Event& event);
        void update_readiness();

    private:
        using ReadyQueue = IntrusiveQueue<Event, &Event::selector_hook_>;

        Event      ready_event_;
        ReadyQueue ready_queue_;
    };

}
