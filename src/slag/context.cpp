#include "context.h"
#include "core.h"
#include "system.h"
#include "application.h"
#include "thread.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    thread_local Context* context_instance = nullptr;

    Context::Context(Application& application, Thread& thread)
        : application_(application)
        , thread_(thread)
    {
        if (context_instance) {
            throw std::runtime_error("Context already bound");
        }

        context_instance = this;
    }

    Context::~Context() {
        context_instance = nullptr;
    }

    Application& Context::application() {
        return application_;
    }

    Thread& Context::thread() {
        return thread_;
    }

    ResourceTable& Context::resource_table(ResourceType type) {
        return thread_.resource_tables()[to_index(type)];
    }

    EventLoop& Context::event_loop() {
        return thread_.event_loop();
    }

    bool has_context() {
        return static_cast<bool>(context_instance);
    }

    Context& get_context() {
        if (!has_context()) {
            abort();
        }

        return *context_instance;
    }

}
