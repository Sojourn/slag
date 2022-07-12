#pragma once

#include <memory>
#include "slag/platform.h"
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
        [[nodiscard]] Reactor& reactor();

    private:
        std::unique_ptr<Platform> platform_;
        std::unique_ptr<Reactor>  reactor_;
        bool                      running_;
    };

    [[nodiscard]] EventLoop& local_event_loop();

}
