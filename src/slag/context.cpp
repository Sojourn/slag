#include "context.h"
#include "core.h"
#include "system.h"
#include "runtime.h"
#include "thread.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    thread_local Context* local_context_instance = nullptr;

    Context::Context(Runtime& runtime)
        : runtime_(runtime)
    {
        if (local_context_instance) {
            throw std::runtime_error("Too many contexts");
        }

        local_context_instance = this;
    }

    Context::~Context() {
        local_context_instance = nullptr;
    }

    Runtime& Context::runtime() {
        return runtime_;
    }

    Thread& Context::thread() {
        if (LIKELY(thread_)) {
            return *thread_;
        }

        throw std::runtime_error("No thread");
    }

    Domain& Context::domain() {
        return runtime_.domain();
    }

    EventLoop& Context::event_loop() {
        if (LIKELY(event_loop_)) {
            return *event_loop_;
        }

        throw std::runtime_error("No event loop");
    }

    Region& Context::region() {
        return event_loop().region();
    }

    Router& Context::router() {
        return event_loop().router();
    }

    Reactor& Context::reactor() {
        return event_loop().reactor();
    }

    void Context::attach(EventLoop& event_loop) {
        assert(!event_loop_);

        event_loop_ = &event_loop;
    }

    void Context::detach(EventLoop& event_loop) {
        assert(event_loop_ == &event_loop);

        event_loop_ = nullptr;
    }

    void Context::attach(Thread& thread) {
        assert(!thread_);

        thread_ = &thread;
    }

    void Context::detach(Thread& thread) {
        assert(thread_ == &thread);

        thread_ = nullptr;
    }

    bool has_context() {
        return static_cast<bool>(local_context_instance);
    }

    Context& get_context() {
        if (LIKELY(has_context())) {
            return *local_context_instance;
        }

        throw std::runtime_error("No context");
    }

    Runtime& get_runtime() {
        return get_context().runtime();
    }

    Domain& get_domain() {
        return get_context().domain();
    }

    Thread& get_thread() {
        return get_context().thread();
    }

    EventLoop& get_event_loop() {
        return get_context().event_loop();
    }

    Region& get_region() {
        return get_context().region();
    }

    Router& get_router() {
        return get_context().router();
    }

    Reactor& get_reactor() {
        return get_context().reactor();
    }

}
