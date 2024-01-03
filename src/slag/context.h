#pragma once

#include <array>
#include "core/resource_types.h"

namespace slag {

    class Application;
    class Thread;

    class ResourceTable;
    class EventLoop;

    // A per-thread context object containing references to
    // important services and data structures.
    class Context {
    public:
        Context(Application& application, Thread& thread);
        ~Context();

        Context(Context&&) = delete;
        Context(const Context&) = delete;
        Context& operator=(Context&&) = delete;
        Context& operator=(const Context&) = delete;

        Application& application();
        Thread& thread();

        ResourceTable& resource_table(ResourceType type);
        EventLoop& event_loop();

    private:
        Application& application_;
        Thread&      thread_;
    };

    bool has_context();
    Context& get_context();

}
