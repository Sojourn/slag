#pragma once

#include "core.h"
#include "system.h"

namespace slag {

    class Thread;

    class EventLoop : private InterruptHandler {
    public:
        EventLoop();
        ~EventLoop();

        EventLoop(EventLoop&&) = delete;
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(EventLoop&&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        Reactor& reactor();

        void loop();
        void stop();

        void schedule(Task& task);
        void schedule(Operation& operation);

    private:
        void handle_interrupt(Interrupt interrupt) override final;

    private:
        Reactor      reactor_;
        bool         looping_;
        TaskPriority current_priority_;
        Executor     high_priority_executor_;
        Executor     idle_priority_executor_;
    };

}
