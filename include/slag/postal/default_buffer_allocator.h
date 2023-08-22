#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_allocator.h"

namespace slag::postal {

    // Allocates storage for buffers using the default system allocator.
    //
    // TODO: think about renaming to SystemBufferAllocator.
    //
    class DefaultBufferAllocator : public BufferAllocator {
    public:
        DefaultBufferAllocator(Region& region)
            : BufferAllocator{region}
        {
        }

        std::span<std::byte> allocate_segment_storage(size_t size) override {
            return {
                new std::byte[size],
                size
            };
        }

        void deallocate_segment_storage(std::span<std::byte> storage) override {
            delete[] storage.data();
        }
    };

}
