#pragma once

#include <span>
#include <bitset>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_queue.h"
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_allocator.h"

namespace slag::postal {

    // This should be used for allocations that are expected to
    // have roughly the same lifespan.
    class CohortBufferAllocator : public BufferAllocator {
    public:
        explicit CohortBufferAllocator(Region& region);
        ~CohortBufferAllocator();

        std::span<std::byte> allocate_segment_storage(size_t size) override;
        void deallocate_segment_storage(std::span<std::byte> storage) override;

    private:
        static constexpr size_t BLOCK_SIZE        =  2 * 1024 * 1024;
        static constexpr size_t CHUNK_SIZE        = 32 * 1024;
        static constexpr size_t TABLE_SIZE        = 256;
        static constexpr size_t BLOCK_CHUNK_COUNT = BLOCK_SIZE / CHUNK_SIZE;

        static constexpr size_t BLOCK_LO_MASK     = BLOCK_SIZE - 1;
        static constexpr size_t BLOCK_HI_MASK     = ~BLOCK_LO_MASK;
        static constexpr size_t CHUNK_LO_MASK     = CHUNK_SIZE - 1;
        static constexpr size_t CHUNK_HI_MASK     = ~CHUNK_LO_MASK;

        struct Chunk {
            std::byte storage[CHUNK_SIZE];
        };

        struct Block {
            Chunk chunks[BLOCK_CHUNK_COUNT];
        };

        struct Table {
            IntrusiveQueueNode node;
            uint64_t           dirty_chunk_mask = 0;
            uint16_t           dirty_chunk_size[BLOCK_CHUNK_COUNT] = {{0}};
        };

        static_assert(sizeof(Block) == BLOCK_SIZE);
        static_assert(sizeof(Chunk) == CHUNK_SIZE);
        static_assert(sizeof(Table) <= TABLE_SIZE);

    private:
        struct Cursor {
            size_t chunk_index = 0;
            size_t table_chunk_index = 0;

            Block* block = nullptr;
            Chunk* chunk = nullptr;
            Table* table = nullptr;
        };

        static Cursor to_cursor(uintptr_t address);
        static Cursor to_cursor(const void* address);

        Cursor allocate_block();
        void deallocate_block(Cursor cursor);

        bool advance_block(Cursor& cursor);
        bool advance_chunk(Cursor& cursor);

        void flush_chunk();

    private:
        using Tables = IntrusiveQueue<Table, &Table::node>;

        Cursor cursor_;
        size_t chunk_offset_;

        Tables empty_tables_;
        Tables partial_tables_;
        Tables saturated_tables_;
    };

}
