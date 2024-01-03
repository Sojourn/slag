#include "event_loop.h"
#include <iostream>

namespace slag {

    EventLoop::EventLoop()
        : reactor_(*this)
        , looping_(false)
        , current_priority_(TaskPriority::IDLE)
    {
    }

    EventLoop::~EventLoop() {
        // TODO: poll the reactor until quiescent.
    }

    void EventLoop::loop() {
        if (looping_) {
            throw std::runtime_error("Already looping");
        }

        looping_ = true;
        while (looping_) {
            bool non_blocking = false;
            non_blocking |= high_priority_executor_.is_runnable();
            non_blocking |= idle_priority_executor_.is_runnable();
            reactor_.poll(non_blocking);

            if (high_priority_executor_.is_runnable()) {
                current_priority_ = TaskPriority::HIGH;
                high_priority_executor_.run();
            }
            else if (idle_priority_executor_.is_runnable()) {
                current_priority_ = TaskPriority::IDLE;
                idle_priority_executor_.run();
            }
        }
    }

    void EventLoop::stop() {
        looping_ = false;
    }

    void EventLoop::schedule(Task& task) {
        TaskPriority priority = task.priority();
        if (priority == TaskPriority::SAME) {
            priority = current_priority_;
        }

        if (priority == TaskPriority::HIGH) {
            high_priority_executor_.schedule(task);
        }
        else {
            idle_priority_executor_.schedule(task);
        }
    }

    void EventLoop::schedule(Operation& operation) {
        reactor_.schedule(operation);
    }

    void EventLoop::handle_interrupt(Interrupt interrupt) {
        (void)interrupt;
    }

}
