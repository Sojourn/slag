#pragma once

#include <utility>
#include <memory>
#include <vector>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace slag {

    template<typename T>
    class PoolAllocator {
    public:
        PoolAllocator(size_t initial_capacity = 0);
        PoolAllocator(PoolAllocator&&) noexcept = delete;
        PoolAllocator(const PoolAllocator&) = delete;
        ~PoolAllocator();

        PoolAllocator& operator=(PoolAllocator&&) noexcept = delete;
        PoolAllocator& operator=(const PoolAllocator&) = delete;

        template<typename... Args>
        [[nodiscard]] T& allocate(Args&&... args);
        void deallocate(T& object);

    private:
        void allocate_block();

    private:
        static constexpr size_t STORAGE_SIZE_ = sizeof(T);
        static constexpr size_t STORAGE_ALIGNMENT_ = alignof(std::max_align_t);
        using Storage = std::aligned_storage_t<STORAGE_SIZE_, STORAGE_ALIGNMENT_>;

        static constexpr size_t STORAGE_BLOCK_SIZE_ = (16 * 1024) / sizeof(Storage);
        static_assert(STORAGE_BLOCK_SIZE_ > 0);
        using StorageBlock = std::array<Storage, STORAGE_BLOCK_SIZE_>;

        std::vector<Storage*>                      unused_storage_;
        std::vector<std::unique_ptr<StorageBlock>> storage_blocks_;
    };

}

#include "pool_allocator.hpp"
