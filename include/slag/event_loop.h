#pragma once

#include <memory>
#include "slag/platform.h"
#include "slag/executor.h"
#include "slag/reactor.h"

namespace slag {

    class EventLoop {
    public:
        EventLoop(std::unique_ptr<Reactor> reactor);
        EventLoop(EventLoop&&) noexcept = delete;
        EventLoop(const EventLoop&) = delete;
        ~EventLoop();

        EventLoop& operator=(EventLoop&&) noexcept = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        void run();
        void stop();

        [[nodiscard]] Platform& platform();
        [[nodiscard]] Executor& executor();
        [[nodiscard]] Reactor& reactor();

    private:
        static constexpr size_t EXECUTOR_REACTOR_RATIO_ = 128;

        std::unique_ptr<Platform> platform_;
        std::unique_ptr<Executor> executor_;
        std::unique_ptr<Reactor>  reactor_;
        bool                      running_;
    };

    [[nodiscard]] std::unique_ptr<EventLoop> make_default_event_loop();
    [[nodiscard]] EventLoop& local_event_loop();

}
