#pragma once

#include <limits>
#include <cstdint>
#include <cstddef>

namespace slag {

    class TokenBucket {
    public:
        static constexpr uint64_t NO_WAIT_NS       = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t INFINITE_WAIT_NS = std::numeric_limits<uint64_t>::max();

        TokenBucket(uint64_t tokens_per_second, uint64_t capacity);

        void update(uint64_t now_ns);

        // Consumes tokens from the bucket if enough are available, and returns
        // how long in nanoseconds until the operation would succeed. This will be
        // NO_WAIT_NS (zero) when consume succeeds, and a non-zero number when it fails.
        // INFINITE_WAIT_NS will be returned if this will never succeed due to capacity < token_count.
        uint64_t consume(uint64_t token_count);

        // A fused update and consume function for convenience.
        uint64_t update_and_consume(uint64_t now_ts, uint64_t token_count);

    private:
        uint64_t tokens_;
        uint64_t remainder_;
        uint64_t tokens_per_second_;
        uint64_t capacity_;
        uint64_t last_update_ns_;
    };

}
