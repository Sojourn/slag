#pragma once

#include <liburing.h>
#include <optional>
#include <cstdint>
#include <cstddef>

namespace slag {

    class event_loop_config {
    public:
        event_loop_config();

        const struct io_uring_params& params() const;

    private:
        struct io_uring_params params_;
    };

    class event_loop {
        event_loop(event_loop&&) = delete;
        event_loop(const event_loop&) = delete;
        event_loop& operator=(event_loop&&) = delete;
        event_loop& operator=(const event_loop&) = delete;

    public:
        event_loop(const event_loop_config& config=event_loop_config());
        ~event_loop();

        void step();
        void loop();
        void stop();

    private:
        struct io_uring ring_;
        bool            looping_;
    };

}
