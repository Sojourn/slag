#include "slag/pollable.h"

slag::Pollable::~Pollable() {
    for (auto it = observers_.begin(); it != observers_.end(); ) {
        Observer& observer = *it;
        it = observers_.erase(it);
        observer.handle_pollable_destroyed(*this);
    }
}

const slag::Pollable::EventMask& slag::Pollable::events() const {
    return events_;
}

void slag::Pollable::add_observer(Observer& observer) {
    observers_.push_back(observer);
}

void slag::Pollable::remove_observer(Observer& observer) {
    observers_.erase(observer);
}

void slag::Pollable::set_event(Event event, bool value) {
    events_.set(to_index(event), value);

    if (value) {
        for (auto it = observers_.begin(); it != observers_.end(); ) {
            Observer& observer = *(it++);
            observer.handle_pollable_event(*this, event);
        }
    }
}

size_t slag::to_index(Pollable::Event event) {
    return static_cast<size_t>(event);
}

const char* slag::to_string(Pollable::Event event) {
    switch (event) {
#define X(SLAG_POLLABLE_EVENT) case Pollable::Event::SLAG_POLLABLE_EVENT: return #SLAG_POLLABLE_EVENT;
        SLAG_POLLABLE_EVENTS(X)
#undef X
    }

    abort();
}

slag::Pollable::EventMask slag::make_pollable_events(std::initializer_list<Pollable::Event> events) {
    Pollable::EventMask result;
    for (Pollable::Event event: events) {
        result.set(to_index(event));
    }

    return result;
}
