#include "slag/awaitable.h"

slag::Awaitable::Awaitable(Pollable& pollable, PollableEventMask events)
    : pollable_{pollable}
    , events_{events}
    , fiber_{nullptr}
{
}

bool slag::Awaitable::await_ready() const noexcept {
    return (pollable_.events() & events_).none();
}

void slag::Awaitable::await_suspend(std::coroutine_handle<> handle) {
    pollable_.add_observer(*this);

    fiber_  = local_active_fiber();
    handle_ = std::move(handle);
}

void slag::Awaitable::handle_pollable_event(Pollable& pollable, Pollable::Event event) {
    assert(&pollable_ == &pollable);

    if (events_.test(to_index(event))) {
        fiber_->resume(std::move(handle_));
    }
}

void slag::Awaitable::handle_pollable_destroyed(Pollable& pollable) {
    assert(&pollable_ == &pollable);

    fiber_->resume(std::move(handle_));
}
