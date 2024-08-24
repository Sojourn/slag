#pragma once

#include <array>

namespace slag {

    class Application;
    class Thread;
    class EventLoop;
    class Reactor;

    // A per-thread context object containing references to
    // important services and data structures.
    class ThreadContext {
    public:
        explicit ThreadContext(Thread& thread);
        ~ThreadContext();

        ThreadContext(ThreadContext&&) = delete;
        ThreadContext(const ThreadContext&) = delete;
        ThreadContext& operator=(ThreadContext&&) = delete;
        ThreadContext& operator=(const ThreadContext&) = delete;

        Application& application();
        Thread& thread();
        EventLoop& event_loop();
        Reactor& reactor();

    private:
        Thread& thread_;
    };

    bool has_context();
    ThreadContext& get_context();

    inline Application& get_application() {
        return get_context().application();
    }

    inline Thread& get_thread() {
        return get_context().thread();
    }

    inline EventLoop& get_event_loop() {
        return get_context().event_loop();
    }

    inline Reactor& get_reactor() {
        return get_context().reactor();
    }

}
