#include "slag/event.h"
#include <algorithm>
#include <cassert>

slag::EventObserver::~EventObserver() {
    for (auto&& [event, _]: event_waits_) {
        assert(event->observer_ == this);
        event->observer_ = nullptr;
    }
}

void slag::EventObserver::wait(Event& event, void* user_data) {
    if (event.observer_) {
        if (event.observer_ != this) {
            assert(find_wait(event) == event_waits_.end());
            throw std::runtime_error("Another EventObserver is already waiting for this Event");
        }

        return; // already waiting for this event
    }

    event.observer_ = this;
    event_waits_.push_back(std::make_pair(&event, user_data));
    if (event.is_set()) {
        handle_event_set(event, user_data);
    }
}

void slag::EventObserver::cancel_wait(Event& event) {
    if (event.observer_ != this) {
        assert(find_wait(event) == event_waits_.end());
        return; // not waiting for this event
    }

    auto it = find_wait(event);
    assert(it != event_waits_.end());
    event_waits_.erase(it);
    event.observer_ = nullptr;
}

auto slag::EventObserver::find_wait(Event& event) -> EventWaits::iterator {
    return std::find_if(
        event_waits_.begin(),
        event_waits_.end(),
        [&](const EventWait& event_wait) {
            return event_wait.first == &event;
        }
    );
}

slag::Event::Event()
    : observer_{nullptr}
    , is_set_{false}
{
}

slag::Event::~Event() {
    if (observer_) {
        auto it = observer_->find_wait(*this);
        assert(it != observer_->event_waits_.end());

        void* user_data = it->second;
        observer_->event_waits_.erase(it);

        EventObserver* observer = observer_;
        observer_ = nullptr;
        observer->handle_event_destroyed(user_data);
    }
}

bool slag::Event::is_set() const {
    return is_set_;
}

void slag::Event::set() {
    if (is_set_) {
        return;
    }

    is_set_ = true;
    if (EventObserver* observer = observer_) {
        auto it = observer->find_wait(*this);
        assert(it != observer->event_waits_.end());
        observer->handle_event_set(*this, it->second);
    }
}

void slag::Event::reset() {
    is_set_ = false;
}
