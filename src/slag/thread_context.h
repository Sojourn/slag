#pragma once

#include <array>

namespace slag {

    class Application;
    class Thread;

    class ResourceTable;
    class EventLoop;

    // A per-thread context object containing references to
    // important services and data structures.
    class ThreadContext {
    public:
        ThreadContext(Application& application, Thread& thread);
        ~ThreadContext();

        ThreadContext(ThreadContext&&) = delete;
        ThreadContext(const ThreadContext&) = delete;
        ThreadContext& operator=(ThreadContext&&) = delete;
        ThreadContext& operator=(const ThreadContext&) = delete;

        Application& application();
        Thread& thread();

        EventLoop& event_loop();

    private:
        Application& application_;
        Thread&      thread_;
    };

    bool has_context();
    ThreadContext& get_context();

}