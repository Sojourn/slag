#include "event_loop.h"
#include "slag/application.h"
#include "slag/thread.h"
#include "slag/memory.h"
#include "slag/system.h"
#include "slag/core.h"
#include <stdexcept>

namespace slag {

    // TODO: Move this into a file. We're going to have a couple of them
    //       so it might be worth thinking about their organization.
    class RegionDriver final : public ProtoTask {
    public:
        RegionDriver(Region& region, Reactor& reactor)
            : region_(region)
            , reactor_(reactor)
            , region_fd_(
                make_file_descriptor(region_.file_descriptor(), FileDescriptor::Ownership::BORROWED)
            )
        {
        }

        void run() override {
            SLAG_PT_BEGIN();

            while (true) {
                if (!poll_op_) {
                    reactor_.schedule_operation(
                        poll_op_.emplace(region_fd_)
                    );
                }

                SLAG_PT_WAIT_READABLE(*poll_op_);
                if (poll_op_->result() < 0) {
                    poll_op_->cancel();
                }
                else {
                    assert(poll_op_->result() & POLLIN);
                    constexpr bool non_blocking = true;
                    region_.step(non_blocking);
                }

                if (poll_op_->is_complete()) {
                    poll_op_.reset();
                }
                else {
                    // The operation is still active (multishot).
                }
            }

            SLAG_PT_END();
        }

    private:
        Region& region_;
        Reactor& reactor_;
        Ref<FileDescriptor> region_fd_;
        std::optional<PollMultishotOperation> poll_op_;
    };

    EventLoop::EventLoop(Thread& thread, std::unique_ptr<Task> init)
        : thread_(thread)
        , region_(thread.application().domain(), *this)
        , reactor_(*this)
        , looping_(false)
        , region_driver_(std::make_unique<RegionDriver>(region_, reactor_))
        , init_(std::move(init))
        , current_priority_(TaskPriority::IDLE)
    {
        high_priority_executor_.schedule(*region_driver_);
        high_priority_executor_.schedule(*init_);
    }

    EventLoop::~EventLoop() {
        // FIXME: Properly shut down the reactor to plug some leaks.
    }

    void EventLoop::loop() {
        if (looping_) {
            throw std::runtime_error("Already looping");
        }

        looping_ = true;
        while (looping_)  {
            // Prime the reactor before (potentially) blocking.
            {
                constexpr bool non_blocking = false;
                region_.step(non_blocking);
            }

            // Submit I/O operations and poll for completions.
            {
                bool non_blocking = false;
                non_blocking |= !looping_;
                non_blocking |= high_priority_executor_.is_runnable();
                non_blocking |= idle_priority_executor_.is_runnable();

                reactor_.poll(non_blocking);
            }

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

    void EventLoop::finalize(ObjectGroup group, std::span<Object*> objects) noexcept {
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
        if (looping_ && file_descriptor) {
            start_operation<CloseOperation>(file_descriptor.release())->daemonize();
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
        (void)interrupt;
    }

}
