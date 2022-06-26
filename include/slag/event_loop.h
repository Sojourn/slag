#pragma once

namespace slag {

    class EventLoop {
    public:
        [[nodiscard]] static EventLoop& local_instance();

        EventLoop();
        EventLoop(EventLoop&&) noexcept = delete;
        EventLoop(const EventLoop&) = delete;
        ~EventLoop();

        EventLoop& operator=(EventLoop&&) noexcept = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        void run();
        void stop();

        [[nodiscard]] Driver& driver();
        [[nodiscard]] const Driver& driver() const;

    private:
        Driver& driver_;
    };

}
