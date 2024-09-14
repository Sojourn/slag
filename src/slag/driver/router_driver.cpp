#include "slag/driver/router_driver.h"
#include "slag/event_loop.h"

namespace slag {

    RouterDriver::RouterDriver(EventLoop& event_loop)
        : rx_worker_(event_loop)
        , tx_worker_(event_loop)
    {
    }

    RouterDriver::RxWorker::RxWorker(EventLoop& event_loop)
        : ProtoTask(TaskPriority::HIGH)
        , event_loop_(event_loop)
        , router_(event_loop.router())
        , interrupt_event_(event_loop.interrupt_vector()[to_index(InterruptReason::LINK)])
    {
    }

    void RouterDriver::RxWorker::run() {
        SLAG_PT_BEGIN();

        while (true) {
            SLAG_PT_WAIT_EVENT(interrupt_event_);
            interrupt_event_.reset();

            // TODO: Avoid going to sleep for ~10 microseconds, which is enough
            //       time for a round trip between threads.
            while (router_.poll()) {
                SLAG_PT_YIELD();
            }
        }

        SLAG_PT_END();
    }

    RouterDriver::TxWorker::TxWorker(EventLoop& event_loop)
        : ProtoTask(TaskPriority::HIGH)
        , event_loop_(event_loop)
        , router_(event_loop_.router())
    {
    }

    void RouterDriver::TxWorker::run() {
        SLAG_PT_BEGIN();

        while (true) {
            SLAG_PT_WAIT_WRITABLE(router_);

            router_.flush();
        }

        SLAG_PT_END();
    }

}
