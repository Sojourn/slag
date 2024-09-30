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
    {
    }

    void RouterDriver::RxWorker::run() {
        SLAG_PT_BEGIN();

        while (true) {
            SLAG_PT_WAIT_READABLE(router_);
            router_.poll();
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
