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

    Thread::Thread(Application& application, std::unique_ptr<Task> init)
        : application_(application)
        , event_loop_(nullptr)
        , init_(std::move(init))
    {
    }

    Thread::~Thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    Application& Thread::application() {
        return application_;
    }

    EventLoop& Thread::event_loop() {
        return *event_loop_;
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

    void Thread::run() {
        EventLoop event_loop(*this, std::move(init_));

        {
            ThreadContext context(application_, *this);

            event_loop_ = &event_loop;
            event_loop.loop();
            event_loop_ = nullptr;
        }
    }

}
