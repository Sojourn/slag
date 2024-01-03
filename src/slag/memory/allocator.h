#pragma once

#include <span>
#include <cstdint>
#include <cstddef>

namespace slag {

    class Allocator {
    public:
        virtual ~Allocator() = default;

        virtual std::span<std::byte> allocate(size_t size_bytes) = 0;
        virtual void deallocate(std::span<std::byte> storage) = 0;
    };

}
