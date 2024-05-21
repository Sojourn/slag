#include "thread_context.h"
#include "core.h"
#include "system.h"
#include "application.h"
#include "thread.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    thread_local ThreadContext* context_instance = nullptr;

    ThreadContext::ThreadContext(Application& application, Thread& thread)
        : application_(application)
        , thread_(thread)
    {
        if (context_instance) {
            throw std::runtime_error("Context already bound");
        }

        context_instance = this;
    }

    ThreadContext::~ThreadContext() {
        context_instance = nullptr;
    }

    Application& ThreadContext::application() {
        return application_;
    }

    Thread& ThreadContext::thread() {
        return thread_;
    }

    EventLoop& ThreadContext::event_loop() {
        return thread_.event_loop();
    }

    bool has_context() {
        return static_cast<bool>(context_instance);
    }

    ThreadContext& get_context() {
        if (!has_context()) {
            abort();
        }

        return *context_instance;
    }

}
