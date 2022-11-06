#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/handle.h"

namespace slag {

    // TODO: allocate these in blocks that are registered w/ io_uring for fixed operations.
    class Buffer {
    public:
        Buffer(size_t capacity);
        Buffer(std::span<const std::byte> data);

        [[nodiscard]] std::span<std::byte> data();
        [[nodiscard]] std::span<const std::byte> data() const;

    private:
        std::vector<std::byte> data_;
    };

}
