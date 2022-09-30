#include "slag/fiber.h"
#include "slag/event_loop.h"

slag::FiberBase::Activation::Activation(FiberBase& fiber)
    : event_loop_{local_event_loop()}
    , previous_active_fiber_{event_loop_.exchange_active_fiber(&fiber)}
{
}

slag::FiberBase::Activation::~Activation() {
    (void)event_loop_.exchange_active_fiber(previous_active_fiber_);
}
