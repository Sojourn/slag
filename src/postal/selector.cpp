#include "slag/postal/selector.h"
#include <cassert>

namespace slag::postal {

    Event& Selector::readable_event() {
        return ready_event_;
    }

    void Selector::insert(Event& event) {
        Event* pointer = &event;
        insert(std::span{&pointer, 1});
    }

    void Selector::insert(std::span<Event*> events) {
        for (Event* event: events) {
            if (!event) {
                continue; // The events array is allowed to be sparse.
            }

            event->attach(*this);

            if (event->is_set()) {
                ready_queue_.push_back(*event);
            }
        }

        update_readiness();
    }

    void Selector::remove(Event& event) {
        Event* pointer = &event;
        remove(std::span{&pointer, 1});
    }

    void Selector::remove(std::span<Event*> events) {
        for (Event* event: events) {
            if (!event) {
                continue; // The events array is allowed to be sparse.
            }

            if (event->is_set()) {
                ready_queue_.erase(*event);
            }
            else {
                // The event is not in our ready queue and does not need to be unlinked,
                // just told that it is dead to us.
            }

            event->detach(*this);
        }

        update_readiness();
    }

    Event* Selector::select() {
        Event* event = ready_queue_.pop_front();
        if (event) {
            event->detach(*this);
        }

        update_readiness();

        return event;
    }

    size_t Selector::select(std::span<Event*> events) {
        size_t count = ready_queue_.pop_front(events);
        for (size_t index = 0; index < count; ++index) {
            events[index]->detach(*this);
        }

        update_readiness();

        return count;
    }

    void Selector::handle_readiness_change(Event& event) {
        if (event.is_set()) {
            ready_queue_.push_back(event);
        }
        else {
            ready_queue_.erase(event);
        }

        update_readiness();
    }

    void Selector::update_readiness() {
        // We are ready (readable) when the queue is not empty.
        ready_event_.set(
            !ready_queue_.is_empty()
        );
    }

}
