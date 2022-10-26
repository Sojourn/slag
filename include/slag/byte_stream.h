#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_list.h"

namespace slag {

    class ByteStream {
    public:
        void write(std::span<const std::byte> data);
        std::span<const std::byte> read(size_t count);

    private:
        struct Segment {
            uint64_t               relative_producer_sequence = 0;
            uint64_t               relative_consumer_sequence = 0;
            std::vector<std::byte> data;
        };

        uint64_t             absolute_producer_sequence_ = 0;
        uint64_t             absolute_consumer_sequence_ = 0;
        std::vector<Segment> segments_;
    };

}
