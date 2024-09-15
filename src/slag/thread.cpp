#include "thread.h"
#include "context.h"
#include "runtime.h"
#include "memory/buffer.h"
#include "system/operation.h"

namespace slag {

    Thread::Thread(Runtime& runtime, const ThreadConfig& config)
        : runtime_(runtime)
        , config_(config)
        , fabric_(runtime_.fabric())
        , reactor_(runtime_.reactor(config_.index))
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    const ThreadConfig& Thread::config() const {
        return config_;
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
