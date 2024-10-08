#include "event_loop.h"
#include "slag/runtime.h"
#include "slag/thread.h"
#include "slag/context.h"
#include "slag/memory.h"
#include "slag/system.h"
#include "slag/core.h"
#include <stdexcept>

namespace slag {

    EventLoop::EventLoop(const ThreadIndex thread_index, Components components)
        : region_(components.domain, *this)
        , router_(std::move(components.fabric), thread_index)
        , reactor_(std::move(components.reactor))
        , current_priority_(TaskPriority::HIGH) // This will give the root task high-priority.
    {
    }

    bool EventLoop::is_running() const {
        return static_cast<bool>(root_task_);
    }

    Region& EventLoop::region() {
        return region_;
    }

    Router& EventLoop::router() {
        return router_;
    }

    Reactor& EventLoop::reactor() {
        return *reactor_;
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

    void EventLoop::run(std::unique_ptr<Task> root_task) {
        if (root_task_) {
            throw std::runtime_error("Already running");
        }

        drivers_.emplace(*this);
        root_task_ = std::move(root_task);

        loop();
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
                    finalize(static_cast<R&>(*object));               \
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

    void EventLoop::finalize(Managed& managed) {
        delete &managed;
    }

    void EventLoop::finalize(Message& message) {
        router_.finalize(message);
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
            reactor_->destroy_operation(operation);
        }
        else {
            operation.cancel();
        }
    }

    void EventLoop::loop() {
        // TODO: Use a feedback loop to calibrate these for different workloads.
        constexpr size_t high_priority_budget = 32;
        constexpr size_t idle_priority_budget = 1;

        while (root_task_ && !root_task_->is_complete())  {
            // Prime the reactor before (potentially) blocking.
            {
                constexpr bool non_blocking = true;
                region_.step(non_blocking);
            }

            // Submit I/O operations and poll for completions.
            {
                bool non_blocking = false;
                non_blocking |= high_priority_executor_.is_runnable();
                non_blocking |= idle_priority_executor_.is_runnable();

                reactor_->poll(non_blocking);
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
    }

}
