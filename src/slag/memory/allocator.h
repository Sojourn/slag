#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/core.h"

namespace slag {

    class Allocator {
        virtual ~Allocator() = default;

        virtual std::span<std::byte> allocate(size_t size_bytes) = 0;
        virtual void deallocate(std::span<std::byte> storage) = 0;
    };

}
