#include "slag/event.h"
#include <algorithm>
#include <cassert>

slag::EventObserver::~EventObserver() {
    for (auto&& [event, _]: event_waits_) {
        assert(event->tagged_observer_pointer_ == this);
        event->tagged_observer_pointer_ = nullptr;
    }
}

void slag::EventObserver::wait(Event& event, void* user_data) {
    if (event.tagged_observer_pointer_) {
        if (event.tagged_observer_pointer_ != this) {
            assert(find_wait(event) == event_waits_.end());
            throw std::runtime_error("Another EventObserver is already waiting for this Event");
        }

        return; // already waiting for this event
    }

    event.tagged_observer_pointer_ = this;
    event_waits_.push_back(std::make_pair(&event, user_data));
    if (event.is_set()) {
        handle_event_set(event, user_data);
    }
}

void slag::EventObserver::cancel_wait(Event& event) {
    if (event.tagged_observer_pointer_ != this) {
        assert(find_wait(event) == event_waits_.end());
        return; // not waiting for this event
    }

    auto it = find_wait(event);
    assert(it != event_waits_.end());
    event_waits_.erase(it);
    event.tagged_observer_pointer_ = nullptr;
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
    : tagged_observer_pointer_{nullptr, static_cast<size_t>(false)}
{
}

slag::Event::~Event() {
    if (tagged_observer_pointer_) {
        auto it = tagged_observer_pointer_->find_wait(*this);
        assert(it != tagged_observer_pointer_->event_waits_.end());

        void* user_data = it->second;
        tagged_observer_pointer_->event_waits_.erase(it);

        EventObserver* observer = tagged_observer_pointer_.pointer();
        tagged_observer_pointer_.set_pointer(nullptr);

        observer->handle_event_destroyed(user_data);
    }
}

bool slag::Event::is_set() const {
    return static_cast<bool>(tagged_observer_pointer_.tag());
}

void slag::Event::set() {
    if (is_set()) {
        return;
    }

    tagged_observer_pointer_.set_tag(static_cast<size_t>(true));
    if (EventObserver* observer = tagged_observer_pointer_.pointer()) {
        auto it = observer->find_wait(*this);
        assert(it != observer->event_waits_.end());
        observer->handle_event_set(*this, it->second);
    }
}

void slag::Event::reset() {
    tagged_observer_pointer_.set_tag(static_cast<size_t>(false));
}
