#include "event_loop.h"
#include "slag/runtime.h"
#include "slag/thread.h"
#include "slag/context.h"
#include "slag/memory.h"
#include "slag/system.h"
#include "slag/core.h"
#include <stdexcept>

namespace slag {

    EventLoop::EventLoop(Domain& domain, std::shared_ptr<Fabric> fabric)
        : region_(domain, *this)
        , router_(std::move(fabric), get_thread().config().index)
        , reactor_(*this)
        , current_priority_(TaskPriority::HIGH) // This will give the root task high-priority.
    {
        get_context().attach(*this);
    }

    EventLoop::~EventLoop() {
        get_context().detach(*this);
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

    InterruptVector& EventLoop::interrupt_vector() {
        return interrupt_vector_;
    }

    void EventLoop::run(std::unique_ptr<Task> root_task) {
        if (root_task_) {
            throw std::runtime_error("Already running");
        }

        region_driver_.emplace(region_, reactor_);
        router_driver_.emplace(*this);

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
            reactor_.destroy_operation(operation);
        }
        else {
            operation.cancel();
        }
    }

    void EventLoop::handle_interrupt(Interrupt interrupt) {
        interrupt_vector_[to_index(interrupt.reason)].set();

        switch (interrupt.reason) {
            case InterruptReason::HALT: {
                constexpr bool force = true;
                stop(force);
                break;
            }
            case InterruptReason::STOP: {
                constexpr bool force = false;
                stop(force);
                break;
            }
            default: {
                break;
            }
        }
    }

    void EventLoop::loop() {
        // TODO: Use a feedback loop to calibrate these for different workloads.
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
