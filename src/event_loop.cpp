#include "slag/event_loop.h"
#include <cassert>

namespace slag {

    EventLoop::EventLoop()
        : reactor_{executor_}
        , running_{true}
    {
    }

    EventLoop::~EventLoop() {
        while (!reactor_.is_quiescent()) {
            bool non_blocking = false;
            reactor_.poll(non_blocking);
        }
    }

    void EventLoop::bind(Task& task) {
        executor_.insert(task);
    }

    void EventLoop::loop() {
        while (running_) {
            step();
        }
    }

    bool EventLoop::step() {
        if (running_ && executor_.is_runnable()) {
            executor_.run();
        }

        // Don't block if:
        //   1. There is more work to do in userspace.
        //   2. We are stopping. Need to return and not block forever to
        //      give services a chance to cancel tasks before we shutdown
        //      our reactor while destructing.
        //
        bool non_blocking = executor_.is_runnable() || !running_;
        reactor_.poll(non_blocking);
        return running_;
    }

    void EventLoop::stop() {
        running_ = false;
    }

}
