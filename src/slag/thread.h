#pragma once

#include "types.h"
#include "core.h"
#include "context.h"
#include "event_loop.h"
#include "memory/buffer.h"
#include "system/operation.h"
#include "topology.h"

#include "mantle/mantle.h"

#include <optional>
#include <future>
#include <span>
#include <string>
#include <memory>
#include <thread>

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

        ThreadIndex index() const;
        const ThreadConfig& config() const;

        Runtime& runtime();
        EventLoop& event_loop();

        template<typename RootTask, typename... Args>
        void run(Args&&... args);

    private:
        Runtime&                   runtime_;
        ThreadConfig               config_;
        std::shared_ptr<Fabric>    fabric_;
        std::shared_ptr<Reactor>   reactor_;
        std::unique_ptr<EventLoop> event_loop_;
        std::thread                thread_;
    };

    template<typename RootTask, typename... Args>
    void Thread::run(Args&&... args) {
        if (thread_.joinable()) {
            throw std::runtime_error("Thread is already running");
        }

        std::promise<void> started_promise;
        std::future<void> started_future = started_promise.get_future();

        thread_ = std::thread([started_promise=std::move(started_promise), this](Args&&... args) mutable {
            Context context(runtime_, *this);

            std::unique_ptr<RootTask> root_task;

            try {
                if (config_.cpu_affinities) {
                    mantle::set_cpu_affinity(*config_.cpu_affinities);
                }

                event_loop_ = std::make_unique<EventLoop>(context.domain(), std::move(fabric_), std::move(reactor_));
                root_task = std::make_unique<RootTask>(std::forward<Args>(args)...);

                started_promise.set_value();
            }
            catch (const std::exception&) {
                started_promise.set_exception(std::current_exception());
            }

            event_loop_->run(std::move(root_task));
            event_loop_.reset();

        }, std::forward<Args>(args)...);

        started_future.get();
    }

}
