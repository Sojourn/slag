#include "slag/postal/cohort_buffer_allocator.h"
#include "slag/memory.h"
#include "slag/util.h"
#include <bit>
#include <cstring>
#include <cassert>

namespace slag::postal {

    CohortBufferAllocator::CohortBufferAllocator(Region& region)
        : BufferAllocator{region}
        , chunk_offset_{CHUNK_SIZE}
    {
    }

    CohortBufferAllocator::~CohortBufferAllocator() {
        if (cursor_.block) {
            deallocate_block(cursor_);
        }

        for (auto&& tables: {&empty_tables_, &partial_tables_, &saturated_tables_}) {
            while (Table* table = tables->pop_front()) {
                deallocate_block(to_cursor(table));
            }
        }
    }

    std::span<std::byte> CohortBufferAllocator::allocate_segment_storage(size_t size) {
        if (!size) {
            return {};
        }

        while (true) {
            if (chunk_offset_ == CHUNK_SIZE) {
                if (!advance_chunk(cursor_)) {
                    cursor_ = allocate_block();
                    chunk_offset_ = 0;
                }
            }

            auto chunk_beg = &cursor_.chunk->storage[0];
            auto chunk_end = chunk_beg + CHUNK_SIZE;
            auto chunk_rng = std::make_pair(chunk_beg, chunk_end);

            auto segment_beg = chunk_beg + chunk_offset_;
            auto segment_end = segment_beg + std::min(size, CHUNK_SIZE - chunk_offset_);
            auto segment_rng = std::make_pair(segment_beg, segment_end);

            // Special logic to avoid the table if it is located in this chunk.
            if (cursor_.chunk_index == cursor_.table_chunk_index) {
                auto table_beg = reinterpret_cast<std::byte*>(cursor_.table);
                auto table_end = table_beg + sizeof(Table);
                auto table_rng = std::make_pair(table_beg, table_end);

                // Attempt to adjust the segment to avoid the table.
                if (is_overlapping(segment_rng, table_rng)) {
                    if (segment_beg < table_beg) {
                        // Squeeze the allocation in before the table.
                        segment_end = table_beg;
                        segment_rng = std::make_pair(segment_beg, segment_end);
                    }
                    else {
                        // Advance past the table and restart the allocation.
                        chunk_offset_ = table_end - chunk_beg;
                        continue;
                    }
                }

                assert(!is_overlapping(segment_rng, table_rng));
            }
            assert(segment_beg < segment_end);
            assert(is_overlapping(segment_rng, chunk_rng));

            std::span<std::byte> storage{segment_beg, static_cast<size_t>(segment_end - segment_beg)};
            chunk_offset_ += storage.size_bytes();
            return storage;
        }
    }

    void CohortBufferAllocator::deallocate_segment_storage(std::span<std::byte> storage) {
        Cursor cursor = to_cursor(storage.data());
        Table& table = *cursor.table;

        auto&& dirty_chunk_size = table.dirty_chunk_size[cursor.chunk_index];
        dirty_chunk_size -= storage.size_bytes();
        if (dirty_chunk_size == 0) {
            bool was_saturated = (~table.dirty_chunk_mask) == 0;

            // Mark this chunk as being clean.
            reset_bit(table.dirty_chunk_mask, cursor_.chunk_index);

            if (cursor_.table == &table) {
                // This block is hot, and being used for allocations.
                assert(!table.node.is_linked());
            }
            else {
                if (was_saturated) {
                    saturated_tables_.erase(table);
                    partial_tables_.push_back(table);
                }
                else if (!table.dirty_chunk_mask) {
                    partial_tables_.erase(table);
                    empty_tables_.push_back(table);
                }
                else {
                    // Still partial.
                    assert(table.node.is_linked());
                }
            }
        }
    }

    auto CohortBufferAllocator::to_cursor(uintptr_t address) -> Cursor {
        Cursor cursor;

        auto block_address = address & BLOCK_HI_MASK;
        auto chunk_address = address & CHUNK_HI_MASK;
        auto block_shape   = (block_address / BLOCK_SIZE) & ((BLOCK_SIZE / TABLE_SIZE) - 1);
        auto table_address = block_address + (block_shape * TABLE_SIZE);

        cursor.chunk_index = (chunk_address - block_address) / CHUNK_SIZE;
        cursor.table_chunk_index = (table_address - block_address) / CHUNK_SIZE;

        cursor.block = reinterpret_cast<Block*>(block_address);
        cursor.chunk = reinterpret_cast<Chunk*>(chunk_address);
        cursor.table = reinterpret_cast<Table*>(table_address);

        return cursor;
    }

    auto CohortBufferAllocator::to_cursor(const void* address) -> Cursor {
        return to_cursor(reinterpret_cast<uintptr_t>(address));
    }

    auto CohortBufferAllocator::allocate_block() -> Cursor {
        std::span<std::byte> block_storage = allocate_huge_pages(sizeof(Block));

        Cursor cursor = to_cursor(block_storage.data());
        new(cursor.table) Table;
        return cursor;
    }

    void CohortBufferAllocator::deallocate_block(Cursor cursor) {
        cursor.table->~Table();

        deallocate_huge_pages(
            std::as_writable_bytes(std::span{cursor.block, 1})
        );
    }

    bool CohortBufferAllocator::advance_block(Cursor& cursor) {
        for (auto&& tables: {&partial_tables_, &empty_tables_}) {
            if (Table* table = tables->pop_front()) {
                cursor = to_cursor(table);
                return true;
            }
        }

        return false;
    }

    bool CohortBufferAllocator::advance_chunk(Cursor& cursor) {
        if (Table* table = cursor.table) {
            flush_chunk();

            if (uint64_t clean_chunk_mask = ~table->dirty_chunk_mask) {
                cursor_.chunk_index = static_cast<size_t>(__builtin_ctzll(clean_chunk_mask));
                chunk_offset_ = 0;
                return true;
            }
        }

        return advance_block(cursor);
    }

    void CohortBufferAllocator::flush_chunk() {
        assert(cursor_.chunk);
        assert(chunk_offset_);

        auto&& table = *cursor_.table;
        auto&& dirty_chunk_mask = table.dirty_chunk_mask;
        auto&& dirty_chunk_size = table.dirty_chunk_size[cursor_.chunk_index];

        set_bit(dirty_chunk_mask, cursor_.chunk_index);;
        dirty_chunk_size = static_cast<uint16_t>(chunk_offset_);
        if (cursor_.chunk_index == cursor_.table_chunk_index) {
            dirty_chunk_size -= TABLE_SIZE;
        }
    }

}
