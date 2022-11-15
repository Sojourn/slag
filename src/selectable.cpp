#include "slag/selectable.h"

slag::Selectable::~Selectable() {
    for (auto it = observers_.begin(); it != observers_.end(); ) {
        Observer& observer = *it;
        it = observers_.erase(it);
        observer.handle_selectable_destroyed(*this);
    }
}

const slag::Selectable::EventMask& slag::Selectable::events() const {
    return events_;
}

void slag::Selectable::add_observer(Observer& observer) {
    observers_.push_back(observer);
}

void slag::Selectable::remove_observer(Observer& observer) {
    observers_.erase(observer);
}

void slag::Selectable::set_event(Event event, bool value) {
    events_.set(to_index(event), value);

    if (value) {
        for (auto it = observers_.begin(); it != observers_.end(); ) {
            Observer& observer = *(it++);
            observer.handle_selectable_event(*this, event);
        }
    }
}

size_t slag::to_index(Selectable::Event event) {
    return static_cast<size_t>(event);
}

const char* slag::to_string(Selectable::Event event) {
    switch (event) {
#define X(SLAG_SELECTABLE_EVENT) case Selectable::Event::SLAG_SELECTABLE_EVENT: return #SLAG_SELECTABLE_EVENT;
        SLAG_SELECTABLE_EVENTS(X)
#undef X
    }

    abort();
}

slag::Selectable::EventMask slag::make_selectable_events(std::initializer_list<Selectable::Event> events) {
    Selectable::EventMask result;
    for (Selectable::Event event: events) {
        result.set(to_index(event));
    }

    return result;
}
