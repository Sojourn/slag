#include "slag/pollable.h"

size_t slag::to_index(PollableEvent event) {
    return static_cast<size_t>(event);
}

const char* slag::to_string(PollableEvent event) {
    switch (event) {
#define X(SLAG_POLLABLE_EVENT) case PollableEvent::SLAG_POLLABLE_EVENT: return #SLAG_POLLABLE_EVENT;
        SLAG_POLLABLE_EVENTS(X)
#undef X
    }

    abort();
}

slag::PollableEventMask::PollableEventMask(PollableEvent event) {
    set(to_index(event));
}

slag::PollableEventMask::PollableEventMask(std::initializer_list<PollableEvent> events) {
    for (auto&& event: events) {
        set(to_index(event));
    }
}

bool slag::PollableEventMask::test(PollableEvent event) const {
    return test(to_index(event));
}

slag::PollableEventMask& slag::PollableEventMask::set(PollableEvent event, bool value) {
    set(to_index(event), value);
    return *this;
}

std::string slag::to_string(PollableEventMask events) {
    std::string result;
    for (size_t i = 0; i < POLLABLE_EVENT_COUNT; ++i) {
        if (events.test(i)) {
            if (!result.empty()) {
                result += '|';
            }

            result += to_string(static_cast<PollableEvent>(i));
        }
    }

    return result;
}

slag::Pollable::~Pollable() {
    for (auto it = observers_.begin(); it != observers_.end(); ) {
        Observer& observer = *it;
        it = observers_.erase(it);
        observer.handle_pollable_destroyed(*this);
    }
}

auto slag::Pollable::events() const -> const EventMask& {
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
