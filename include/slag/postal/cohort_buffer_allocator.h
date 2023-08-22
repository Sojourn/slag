#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_allocator.h"

namespace slag::postal {

    // This should be used for allocations that are expected to
    // have roughly the same lifespan.
    class CohortBufferAllocator : public BufferAllocator {
    public:
        CohortBufferAllocator(Region& region)
            : BufferAllocator{region}
        {
        }

        ~CohortBufferAllocator() {
        }

        std::span<std::byte> allocate_segment_storage(size_t size) override;

        void deallocate_segment_storage(std::span<std::byte> storage) override {
            auto&& [block, chunk] = lookup_allocation(storage.data());

            chunk->reference_count -= 1;
            if (chunk->reference_count == 0) {
            }
        }

    private:
        std::pair<Block*, Chunk*> lookup_allocation(void* address);

    private:
        static constexpr size_t BLOCK_CAPACITY = 64;
        static constexpr size_t CHUNK_CAPACITY = 32 * 1024;

        // Not allocating out of the block itself helps make memory reuse faster.
        struct Chunk {
            size_t reference_count;
            std::array<std::byte, CHUNK_CAPACITY> storage;
        };

        // Unit of allocation and deallocation. Should fit into a huge page (2MB).
        struct Block {
            uint64_t                          chunk_mask;
            std::array<Chunk, BLOCK_CAPACITY> chunk_array;
        };

        // A collection of blocks that all have the same number of free chunks.
        using Bucket = std::vector<Block*>;

        // A bitset for non-empty buckets.
        uint64_t bucket_mask_;

        // Should blocks be in an intrusive queue? That would give us O(1) removal
        // for when things get deallocated.
        std::array<std::vector<Block*>, 64> buckets_;

        // Fifo gives us the best chance of all chunks in a block being
        // unreferenced, and allowing us to free the block. This can probably
        // get destabalized. Might be good to periodically introduce fresh blocks
        // to let things defragment. Is there a fragmentation/memory curve that
        // we can make tradeoffs around?

        // Could also choose based on the max-heap on the popcount of chunk_mask.
        // That would keep us as dense as possible, but chunk selection would be O(log(n)).
        // Might be able have fullness 'buckets', or do some sort of radix sort since we
        // know the cardinality (64).
        Queue<Chunk*>       chunks_;
        std::vector<Block*> blocks_;
    };

}
