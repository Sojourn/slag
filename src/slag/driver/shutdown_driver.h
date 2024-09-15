#pragma once

#include "slag/context.h"
#include "slag/object.h"
#include "slag/core.h"
#include "slag/system/reactor.h"

#include <cassert>

namespace slag {

    // Monitors shutdown conditions and stops the event loop
    // when they are met.
    class ShutdownDriver final : public ProtoTask {
    public:
        explicit ShutdownDriver(EventLoop& event_loop);

        void run() override;

    private:
        EventLoop&      event_loop_;
        InterruptState& stop_state_;
        InterruptState& halt_state_;

        Selector        selector_;
    };

}
