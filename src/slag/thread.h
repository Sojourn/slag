#pragma once

#include <future>
#include <memory>
#include <thread>
#include "types.h"
#include "core.h"
#include "context.h"
#include "mantle/mantle.h"
#include "event_loop.h"
#include "memory/buffer.h"
#include "system/operation.h"

namespace slag {

    class Runtime;

    class Thread {
    public:
        Thread(Runtime& runtime);
        ~Thread();

        Thread(Thread&&) = delete;
        Thread(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

        Runtime& runtime();
        EventLoop& event_loop();

        template<typename TaskImpl, typename... Args>
        void run(Args&&... args);

    private:
        Runtime&     runtime_;
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
                Context context(runtime_);
                context.attach(*this);

                EventLoop event_loop(context.domain());
                {
                    event_loop_ = &event_loop;
                    event_loop_->run<TaskImpl>(std::forward<Args>(args)...);
                    event_loop_ = nullptr;
                }
                context.detach(*this);
            }
            catch (const std::exception& ex) {
                std::cerr << ex.what() << std::endl;
                abort();
            }
        }, std::forward<Args>(args)...);
    }

}
