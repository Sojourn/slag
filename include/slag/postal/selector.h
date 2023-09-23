#pragma once

#include <span>
#include <cstddef>
#include "slag/collection/intrusive_queue.h"
#include "slag/postal/event.h"
#include "slag/postal/pollable.h"

namespace slag::postal {

    class Selector : public Pollable<PollableType::READABLE> {
    public:
        Event& readable_event() override;

        template<PollableType type, typename T>
        void insert(T& object);
        void insert(Event& event);
        void insert(Event& event, void* user_data);
        void insert(std::span<Event*> events); // Can be sparse.

        // TODO: a splice call for doing an insert.

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

    template<PollableType type, typename T>
    inline void Selector::insert(T& object) {
        insert(get_pollable_event<type>(object), &object);
    }

}
