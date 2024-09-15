#pragma once

#include <chrono>
#include <optional>

#include <cstdint>
#include <cstddef>

namespace slag {

    class BasicTimer {
    public:
        template<typename Rep, typename Period>
        void set(const std::chrono::duration<Rep, Period>& duration) {
            expiration_ = std::chrono::steady_clock::now() + duration;
        }

        void reset() {
            expiration_ = std::nullopt;
        }

        bool is_expired() const {
            return expiration_ && (*expiration_ <= std::chrono::steady_clock::now());
        }

    private:
        std::optional<std::chrono::time_point<std::chrono::steady_clock>> expiration_;
    };

}
