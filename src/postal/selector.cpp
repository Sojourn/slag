#include "slag/postal/selector.h"
#include <cassert>

namespace slag::postal {

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
                queue_.push_back(*event);
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
                queue_.erase(*event);
            }

            event->detach(*this);
        }

        update_readiness();
    }

    Event* Selector::select() {
        Event* event = queue_.pop_front();
        if (event) {
            event->detach(*this);
        }

        update_readiness();

        return event;
    }

    size_t Selector::select(std::span<Event*> events) {
        size_t count = queue_.pop_front(events);
        for (size_t index = 0; index < count; ++index) {
            events[index]->detach(*this);
        }

        update_readiness();

        return count;
    }

    void Selector::handle_readiness_change(Event& event) {
        if (event.is_set()) {
            queue_.push_back(event);
        }
        else {
            queue_.erase(event);
        }

        update_readiness();
    }

    void Selector::update_readiness() {
        // We are ready (readable) when the queue is not empty.
        event_.set(
            !ready_queue_.is_empty()
        );
    }

}
