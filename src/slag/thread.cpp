#include "thread.h"
#include "context.h"
#include "runtime.h"
#include "memory/buffer.h"
#include "system/operation.h"

namespace slag {

    Thread::Thread(Runtime& runtime)
        : runtime_(runtime)
        , event_loop_(nullptr)
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    Runtime& Thread::runtime() {
        return runtime_;
    }

    EventLoop& Thread::event_loop() {
        if (LIKELY(event_loop_)) {
            return *event_loop_;
        }

        abort();
    }

}
