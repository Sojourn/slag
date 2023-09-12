#include "slag/token_bucket.h"
#include <cassert>

namespace slag {

    static constexpr uint64_t NANOSECONDS_PER_SECOND = 1000000000;

    TokenBucket::TokenBucket(uint64_t tokens_per_second, uint64_t capacity)
        : tokens_{0}
        , remainder_{0}
        , tokens_per_second_{tokens_per_second}
        , capacity_{capacity}
        , last_update_ns_{0}
    {
    }

    void TokenBucket::update(uint64_t now_ns) {
        if (last_update_ns_ < now_ns) {
            if (last_update_ns_) {
                uint64_t elapsed_ns = now_ns - last_update_ns_;
                uint64_t scaled_token_delta = (elapsed_ns * tokens_per_second_) + remainder_;

                tokens_ += scaled_token_delta / NANOSECONDS_PER_SECOND;
                remainder_  = scaled_token_delta % NANOSECONDS_PER_SECOND;

                if (tokens_ > capacity_) {
                    tokens_ = capacity_;
                    remainder_ = 0;
                }
            }

            last_update_ns_ = now_ns;
        }
    }

    uint64_t TokenBucket::consume(uint64_t token_count) {
        if (tokens_ >= token_count) {
            tokens_ -= token_count;
            return NO_WAIT_NS;
        }
        if (capacity_ < token_count) {
            return INFINITE_WAIT_NS;
        }

        // How many tokens do we need to wait for?
        uint64_t scaled_missing_tokens = (token_count - tokens_) * NANOSECONDS_PER_SECOND;

        // Take partial progress towards the next token into account.
        scaled_missing_tokens -= remainder_;

        // Calculate how many nanoseconds until and update would cause this to succeed.
        return scaled_missing_tokens / tokens_per_second_;
    }

    uint64_t TokenBucket::update_and_consume(uint64_t now_ts, uint64_t token_count) {
        update(now_ts);
        return consume(token_count);
    }

}
