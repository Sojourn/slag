#include "thread.h"
#include "thread_context.h"
#include "application.h"
#include "memory/buffer.h"
#include "system/operation.h"
#include <stdexcept>
#include <iostream>
#include <cstdlib>

namespace slag {

    // TODO: Use a polling operation so we don't need to spin.
    class RegionDriver : public Task {
    public:
        explicit RegionDriver(mantle::Region& region)
            : Task(TaskPriority::IDLE)
            , region_(region)
        {
            runnable_event_.set();
        }

        Event& runnable_event() override final {
            return runnable_event_;
        }

        void run() override final {
            bool non_blocking = true;
            region_.step(non_blocking);
        }

    private:
        mantle::Region& region_;
        Event           runnable_event_;
    };

    Thread::Thread(Application& application, std::unique_ptr<Task> task)
        : application_(application)
        , finalizer_(nullptr)
        , event_loop_(nullptr)
        , root_task_(std::move(task))
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void Thread::start() {
        if (thread_.joinable()) {
            throw std::runtime_error("Thread is already running");
        }

        thread_ = std::thread([this]() {
            try {
                run();
            }
            catch (const std::exception&) {
                // TODO: Log the exception as an error before stopping the thread.
            }

            application_.handle_stopped(*this);
        });
    }

    EventLoop& Thread::event_loop() {
        return *event_loop_;
    }

    void Thread::run() {
        mantle::Region region(application_.domain(), *this);
        EventLoop event_loop;

        RegionDriver region_driver(region);
        event_loop.schedule(region_driver);
        event_loop.schedule(*root_task_);

        {
            ThreadContext context(application_, *this);
            event_loop_ = &event_loop;

            event_loop.loop();

            event_loop_ = nullptr;
        }

        // Explicitly stop the event loop to ensure that all resources are finalized.
        region.stop();
    }

    void Thread::finalize(mantle::Object& object) noexcept {
        if (object.user_data() < static_cast<uint16_t>(RESOURCE_TYPE_COUNT)) {
            ResourceBase& resource_base = static_cast<ResourceBase&>(object);
            switch (resource_base.resource_type()) {
                case ResourceType::BUFFER: {
                    finalize(static_cast<Buffer&>(resource_base));
                    break;
                }
                case ResourceType::OPERATION: {
                    finalize(static_cast<Operation&>(resource_base));
                    break;
                }
            }
        }
        else if (finalizer_) {
            finalizer_->finalize(object);
        }
        else {
            abort();
        }
    }

    void Thread::finalize(Buffer&) noexcept {
        abort(); // TODO
    }

    void Thread::finalize(Operation& operation) noexcept {
        assert(event_loop_);

        Reactor& reactor = event_loop_->reactor();
        reactor.finalize(operation);
    }

}
