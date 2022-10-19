#include "slag/awaitable_base.h"
#include "slag/fiber_base.h"
#include "slag/logging.h"

slag::AwaitableBase::AwaitableBase(Event& event)
    : event_{event}
{
}

bool slag::AwaitableBase::await_ready() const noexcept {
    return event_.is_set();
}

void slag::AwaitableBase::await_suspend(std::coroutine_handle<> handle) {
    handle_ = handle;
    wait(event_, local_active_fiber());
}

void slag::AwaitableBase::handle_event_set(Event& event, void* user_data) {
    (void)event;

    FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
    fiber->resume(handle_);
}

void slag::AwaitableBase::handle_event_destroyed(void* user_data) {
    if (!event_.is_set()) {
        FiberBase* fiber = reinterpret_cast<FiberBase*>(user_data);
        fiber->resume(handle_);
    }
}
