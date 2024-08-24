#include "thread.h"
#include "thread_context.h"
#include "application.h"
#include "memory/buffer.h"
#include "system/operation.h"

namespace slag {

    Thread::Thread(Application& application)
        : application_(application)
        , event_loop_(*this)
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    Application& Thread::application() {
        return application_;
    }

    EventLoop& Thread::event_loop() {
        return event_loop_;
    }

}
