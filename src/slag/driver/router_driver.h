#pragma once

#include "slag/object.h"
#include "slag/core.h"
#include "slag/bus.h"

#include <cassert>

namespace slag {

    class EventLoop;

    class RouterDriver {
    public:
        explicit RouterDriver(EventLoop& event_loop);

    private:
        class RxWorker final : public ProtoTask {
        public:
            explicit RxWorker(EventLoop& event_loop);

            void run() override;

        private:
            EventLoop& event_loop_;
            Router&    router_;
            Event&     interrupt_event_;
        };

        class TxWorker final : public ProtoTask {
        public:
            explicit TxWorker(EventLoop& event_loop);

            void run() override;

        private:
            EventLoop& event_loop_;
            Router&    router_;
        };

    private:
        RxWorker rx_worker_;
        TxWorker tx_worker_;
    };

}
