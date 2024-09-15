#include "shutdown_driver.h"
#include "slag/event_loop.h"

namespace slag {

    ShutdownDriver::ShutdownDriver(EventLoop& event_loop)
        : event_loop_(event_loop)
        , stop_state_(event_loop.reactor().interrupt_state(InterruptReason::STOP))
        , halt_state_(event_loop.reactor().interrupt_state(InterruptReason::HALT))
    {
    }

    void ShutdownDriver::run() {
        SLAG_PT_BEGIN();

        selector_.insert(stop_state_.event);
        selector_.insert(halt_state_.event);

        while (true) {
            SLAG_PT_WAIT_READABLE(selector_);

            while (Event* event = selector_.select()) {
                (void)event;

                if (stop_state_.event) {
                    event_loop_.stop(false); // Cancel the root task and cause a graceful shutdown.
                    stop_state_.event.reset();
                }
                if (halt_state_.event) {
                    event_loop_.stop(true); // Destroy the root task and cause a forceful shutdown.
                    halt_state_.event.reset();
                }
            }
        }

        SLAG_PT_END();
    }

}
