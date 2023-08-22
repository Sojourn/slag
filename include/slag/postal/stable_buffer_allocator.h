#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_allocator.h"

namespace slag::postal {

    // Allocate fixed size chunks of storage out of a provided pool of storage.
    // This is intended to be used for fixed io_uring buffers.
    class StableBufferAllocator : public BufferAllocator {
    public:
        StableBufferAllocator(Region& region, size_t allocation_size, std::span<std::byte> storage)
            : BufferAllocator{region}
        {
        }

        ~StableBufferAllocator() {
        }

        std::span<std::byte> allocate_segment_storage(size_t size_hint) override {
            (void)size_hint; // Don't care. You'll get what you get.

            return {};
        }

        void deallocate_segment_storage(std::span<std::byte> storage) override {
        }

    private:
        size_t               allocation_size_;
        std::span<std::byte> storage_;
        std::vector<size_t>  free_chunks_;
    };

}
