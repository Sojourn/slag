#pragma once

#include <array>
#include "slag/object.h"

namespace slag {

    class Runtime;
    class Thread;
    class EventLoop;
    class Router;
    class Reactor;

    // A directory of important runtime and thread services.
    class Context {
    public:
        explicit Context(Runtime& runtime);
        ~Context();

        Context(Context&&) = delete;
        Context(const Context&) = delete;
        Context& operator=(Context&&) = delete;
        Context& operator=(const Context&) = delete;

        Runtime& runtime();
        Domain& domain();
        Thread& thread();
        EventLoop& event_loop();
        Region& region();
        Router& router();
        Reactor& reactor();

    private:
        friend class Thread;

        void attach(Thread& thread);
        void detach(Thread& thread);

    private:
        friend class EventLoop;

        void attach(EventLoop& event_loop);
        void detach(EventLoop& event_loop);

    private:
        Runtime&   runtime_;
        Thread*    thread_;
        EventLoop* event_loop_;
    };

    bool has_context();
    Context& get_context();
    Runtime& get_runtime();
    Domain& get_domain();
    Thread& get_thread();
    EventLoop& get_event_loop();
    Region& get_region();
    Router& get_router();
    Reactor& get_reactor();

}
