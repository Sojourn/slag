#pragma once

#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag::postal {

    class BufferHandle {
    public:
        BufferHandle();
        BufferHandle(BufferHandle&& other);
        BufferHandle(const BufferHandle&) = delete;
        ~BufferHandle();

        BufferHandle& operator=(BufferHandle&& other);
        BufferHandle& operator=(const BufferHandle&) = delete;

        size_t capacity() const;
        size_t size() const;

        bool is_empty() const;
        bool is_pinned() const;
        bool is_frozen() const;
        bool is_shared() const;

        void pin();
        void unpin();
        void freeze();

        // Allocate a new underlying buffer with the same contents of this.
        // It will initially not be shared/pinned/frozen.
        BufferHandle clone();

        // Return a new handle to the same underlying buffer. A shared
        // buffer is implicitly pinned and frozen.
        BufferHandle share();

        // Return a descriptor for the underlying buffer.
        BufferDescriptor descriptor() const;

        void reset();

    private:
        friend class BufferManager;

        explicit BufferHandle(BufferDescriptor descriptor);

    private:
        BufferDescriptor descriptor_;
    };

    enum class BufferAllocationPolicy {
        ARENA,
        POOL,
    };

    class BufferManager {
    public:
        BufferManager();

    private:
        struct BufferGroupMember {
            std::span<std::byte> buffer;
            BufferDescriptor     descriptor;

            BufferHandle*        handle = nullptr;
            uint32_t             counter_index = 0;
        };

        // Each buffer group will have an allocation policy. Buffers
        // within a group may have different lengths (bump pointer) or
        // the same (pooled).
        struct BufferGroup {
            std::vector<BufferGroupMember> members;
            std::vector<uint32_t>          unused_members;
        };

        std::array<BufferGroup, BUFFER_GROUP_COUNT> groups_;
    };

    constexpr inline uint16_t to_scaled_capacity(size_t capacity) {
        return static_cast<uint16_t>(capacity - BUFFER_CAPACITY_STRIDE) / BUFFER_CAPACITY_STRIDE;
    }

    constexpr inline size_t from_scaled_capacity(uint16_t scaled_capacity) {
        return (static_cast<size_t>(scaled_capacity) * BUFFER_CAPACITY_STRIDE) + BUFFER_CAPACITY_STRIDE;
    }

}
