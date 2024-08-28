#pragma once

#include <optional>
#include <span>
#include <string>
#include <memory>
#include <thread>
#include "types.h"
#include "core.h"
#include "context.h"
#include "mantle/mantle.h"
#include "event_loop.h"
#include "memory/buffer.h"
#include "system/operation.h"
#include "topology.h"

namespace slag {

    class Runtime;

    struct ThreadConfig {
        ThreadIndex index = INVALID_THREAD_INDEX;
        std::optional<std::string> name;
        std::optional<std::span<size_t>> cpu_affinities = std::nullopt;
    };

    class Thread {
    public:
        Thread(Runtime& runtime, const ThreadConfig& config);
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
        ThreadConfig config_;
        std::thread  thread_;
    };

    template<typename TaskImpl, typename... Args>
    void Thread::run(Args&&... args) {
        if (thread_.joinable()) {
            throw std::runtime_error("Thread is already running");
        }

        thread_ = std::thread([this](Args&&... args) {
            try {
                if (config_.cpu_affinities) {
                    mantle::set_cpu_affinity(*config_.cpu_affinities);
                }

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
