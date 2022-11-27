#pragma once

#include <deque>
#include "slag/awaitable.h"

namespace slag {

    template<typename T>
    class Stream : public Pollable {
    public:
        void push(T value) {
            queue_.push_back(std::move(value));
            set_event(Event::READABLE, true);
        }

        [[nodiscard]] std::optional<T> pop() {
            if (queue_.empty()) {
                return std::nullopt;
            }

            std::optional<T> result = std::move(queue_.front());
            queue_.pop_front();
            if (queue_.empty()) {
                set_event(Event::READABLE, false);
            }

            return result;
        }

    private:
        std::deque<T> queue_;
    };

}
