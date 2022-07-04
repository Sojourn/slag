#pragma once

#include <memory>
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

        [[nodiscard]] Reactor& reactor();
        [[nodiscard]] const Reactor& reactor() const;

    private:
        std::unique_ptr<Reactor> reactor_;
        bool                     running_;
    };

    [[nodiscard]] EventLoop& local_event_loop();

}
