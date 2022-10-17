#include "slag/fiber_base.h"
#include "slag/event_loop.h"
#include "slag/logging.h"
#include <cassert>

bool slag::FiberBase::is_done() const {
    return completion().is_set();
}

void slag::FiberBase::resume(std::coroutine_handle<> handle, TaskPriority priority) {
    assert(handle);
    assert(!pending_handle_);

    trace("FiberBase:{} resuming {}", static_cast<void*>(this), handle.address());
    pending_handle_ = handle;
    schedule(priority);
}

void slag::FiberBase::run() {
    if (completion().is_set()) {
        return;
    }

    Activation activation{*this};

    trace("FiberBase:{} running {}", static_cast<void*>(this), pending_handle_.address());
    std::coroutine_handle<> handle = std::exchange(pending_handle_, nullptr);
    handle.resume();

    trace("FiberBase:run exit");
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
