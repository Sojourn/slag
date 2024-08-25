#pragma once

#include <future>
#include <memory>
#include <thread>
#include "types.h"
#include "core.h"
#include "mantle/mantle.h"
#include "event_loop.h"
#include "memory/buffer.h"
#include "system/operation.h"
#include "thread_context.h"

namespace slag {

    class Application;

    class Thread {
    public:
        Thread(Application& application);
        ~Thread();

        Thread(Thread&&) = delete;
        Thread(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

        Application& application();
        EventLoop& event_loop();

        template<typename TaskImpl, typename... Args>
        void run(Args&&... args);

    private:
        Application& application_;
        EventLoop*   event_loop_;
        std::thread  thread_;
    };

    template<typename TaskImpl, typename... Args>
    void Thread::run(Args&&... args) {
        if (thread_.joinable()) {
            throw std::runtime_error("Thread is already running");
        }

        thread_ = std::thread([this](Args&&... args) {
            try {
                ThreadContext context(*this);
                EventLoop event_loop(*this);
                {
                    event_loop_ = &event_loop;
                    event_loop_->run<TaskImpl>(std::forward<Args>(args)...);
                    event_loop_ = nullptr;
                }
            }
            catch (const std::exception& ex) {
                std::cerr << ex.what() << std::endl;
                abort();
            }
        }, std::forward<Args>(args)...);
    }

}
