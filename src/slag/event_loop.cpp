#include "event_loop.h"
#include "slag/application.h"
#include "slag/thread.h"
#include "slag/memory.h"
#include "slag/system.h"
#include "slag/core.h"
#include <stdexcept>

namespace slag {

    EventLoop::EventLoop(Thread& thread)
        : thread_(thread)
        , region_(thread.application().domain(), *this)
        , reactor_(*this)
        , current_priority_(TaskPriority::IDLE)
    {
    }

    EventLoop::~EventLoop() {
        // FIXME: Properly shut down the reactor to plug some leaks.
    }

    bool EventLoop::is_running() const {
        return static_cast<bool>(root_task_);
    }

    Reactor& EventLoop::reactor() {
        return reactor_;
    }

    Executor& EventLoop::executor(TaskPriority priority) {
        if (priority == TaskPriority::SAME) {
            priority = current_priority_;
        }

        switch (priority) {
            case TaskPriority::HIGH: {
                return high_priority_executor_;
            }
            case TaskPriority::IDLE: {
                return idle_priority_executor_;
            }
            default: {
                __builtin_unreachable();
            }
        }
    }

    void EventLoop::stop(bool force) {
        if (root_task_) {
            if (force) {
                root_task_->kill();
            }
            else {
                root_task_->cancel();
            }
        }
    }

    void EventLoop::finalize(ObjectGroup group, std::span<Object*> objects) noexcept {
        // TODO: Defer this and have an `GarbageCollectorDriver` do it.

        // Resources are finalize in batches to improve I-Cache utilization.
        switch (static_cast<ResourceType>(group)) {
#define X(SLAG_RESOURCE_TYPE)                                         \
            case ResourceType::SLAG_RESOURCE_TYPE: {                  \
                using R = Resource<ResourceType::SLAG_RESOURCE_TYPE>; \
                for (Object* object : objects) {                      \
                    finalize(static_cast<R&>(*object));  \
                }                                                     \
                break;                                                \
            }                                                         \

            SLAG_RESOURCE_TYPES(X)
#undef X

            default: {
                abort();
                break;
            }
        }
    }

    void EventLoop::finalize(Buffer& buffer) {
        // TODO: Manage buffer pools.
        delete &buffer;
    }

    void EventLoop::finalize(FileDescriptor& file_descriptor) {
        if (is_running() && file_descriptor) {
            start_close_operation(file_descriptor.release())->daemonize();
        }

        delete &file_descriptor;
    }

    void EventLoop::finalize(Operation& operation) {
        operation.abandon();

        if (operation.is_daemonized()) {
            // This is a cleanup operation or something that we don't want to cancel.
        }
        else if (operation.is_quiescent()) {
            reactor_.destroy_operation(operation);
        }
        else {
            operation.cancel();
        }
    }

    void EventLoop::handle_interrupt(Interrupt interrupt) {
        interrupt_events_[to_index(interrupt.reason)].set();
    }

    void EventLoop::loop() {
        constexpr size_t high_priority_budget = 32;
        constexpr size_t idle_priority_budget = 1;

        while (root_task_ && !root_task_->is_complete())  {
            // Prime the reactor before (potentially) blocking.
            {
                constexpr bool non_blocking = false;
                region_.step(non_blocking);
            }

            // Submit I/O operations and poll for completions.
            {
                bool non_blocking = false;
                non_blocking |= high_priority_executor_.is_runnable();
                non_blocking |= idle_priority_executor_.is_runnable();

                reactor_.poll(non_blocking);
            }

            // Execute tasks for awhile.
            if (high_priority_executor_.is_runnable()) {
                current_priority_ = TaskPriority::HIGH;
                high_priority_executor_.run(high_priority_budget);
            }
            else if (idle_priority_executor_.is_runnable()) {
                current_priority_ = TaskPriority::IDLE;
                idle_priority_executor_.run(idle_priority_budget);
            }
        }

        // Cleanup.
        region_driver_.reset();
    }

}
