#include "thread.h"
#include "context.h"
#include "runtime.h"
#include "memory/buffer.h"
#include "system/operation.h"

namespace slag {

    Thread::Thread(Runtime& runtime, const ThreadIndex index, const ThreadConfig& config)
        : runtime_(runtime)
        , index_(index)
        , config_(config)
        , fabric_(runtime_.fabric())
        , reactor_(runtime_.reactor(index))
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    ThreadIndex Thread::index() const {
        return index_;
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
