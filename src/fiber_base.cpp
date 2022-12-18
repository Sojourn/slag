#include "slag/fiber_base.h"
#include "slag/event_loop.h"
#include "slag/logging.h"
#include <cassert>

bool slag::FiberBase::is_done() const {
    return events().test(Event::READABLE);
}

void slag::FiberBase::resume(std::coroutine_handle<> handle, TaskPriority priority) {
    assert(handle);
    assert(!pending_handle_);

    trace("FiberBase:{} resuming {}", static_cast<void*>(this), handle.address());
    pending_handle_ = handle;
    schedule(priority);
}

void slag::FiberBase::run() {
    if (is_done()) {
        assert(false); // spurious wakeup?
        return;
    }

    {
        Activation activation{*this};

        std::coroutine_handle<> handle = std::exchange(pending_handle_, nullptr);
        handle.resume();
    }
}

slag::FiberBase::Activation::Activation()
    : event_loop_{local_event_loop()}
    , previous_active_fiber_{event_loop_.exchange_active_fiber(nullptr)}
{
}

slag::FiberBase::Activation::Activation(FiberBase& fiber)
    : event_loop_{local_event_loop()}
    , previous_active_fiber_{event_loop_.exchange_active_fiber(&fiber)}
{
}

slag::FiberBase::Activation::~Activation() {
    (void)event_loop_.exchange_active_fiber(previous_active_fiber_);
}

slag::FiberBase* slag::local_active_fiber() {
    return local_event_loop().active_fiber();
}
