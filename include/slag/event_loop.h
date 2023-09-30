#pragma once

#include "slag/core/task.h"
#include "slag/core/executor.h"
#include "slag/system/reactor.h"

namespace slag {

    class EventLoop {
        EventLoop(EventLoop&&) = delete;
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(EventLoop&&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

    public:
        EventLoop();
        ~EventLoop();

        // Bind a root-task.
        void bind(Task& task);

        // Run until a stop is requested.
        void loop();

        // Advance the event loop one step. Returns true if the loop is still running.
        bool step();

        // Request that the event loop stops soon.
        void stop();

    private:
        Executor executor_;
        Reactor  reactor_;
        bool     running_;
    };

}
