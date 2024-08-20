#pragma once

#include <memory>
#include "core.h"
#include "system.h"

namespace slag {

    class Thread;

    class EventLoop : private InterruptHandler {
    public:
        explicit EventLoop(Region& region);
        ~EventLoop();

        EventLoop(EventLoop&&) = delete;
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(EventLoop&&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        void loop();
        void stop();

        void schedule(Task& task);

        template<typename OperationImpl, typename... Args>
        Ref<OperationImpl> start_operation(Args&&... args);

        void finalize(Buffer& buffer);
        void finalize(FileDescriptor& file_descriptor);
        void finalize(Operation& operation);

    private:
        void handle_interrupt(Interrupt interrupt) override final;

    private:
        Region&      region_;
        Reactor      reactor_;
        bool         looping_;
        TaskPriority current_priority_;
        Executor     high_priority_executor_;
        Executor     idle_priority_executor_;
    };

    template<typename OperationImpl, typename... Args>
    Ref<OperationImpl> EventLoop::start_operation(Args&&... args) {
        return reactor_.create_operation<OperationImpl>(std::forward<Args>(args)...);
    }

}
