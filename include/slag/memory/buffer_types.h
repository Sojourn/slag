#pragma once

#include <compare>
#include <cstdint>
#include <cstddef>

namespace slag {

    struct BufferStorage {
        void*   base = nullptr;
        int32_t size = 0;
        int32_t capacity = 0;
        void*   user_data = nullptr;
    };

    struct BufferSegment {
        BufferSegment* next = nullptr;
        BufferStorage  storage;
    };

    struct BufferDescriptor {
        BufferIndex buffer_index = INVALID_BUFFER_INDEX;
        ThreadIndex thread_index = INVALID_THREAD_INDEX;

        constexpr auto operator<=>(const BufferDescriptor&) const = default;

        explicit operator bool() const {
            return *this != BufferDescriptor{};
        }
    };

    // Should this be a class?
    struct BufferDescription {
        BufferSegment    head;
        BufferSegment*   tail = nullptr;
        int32_t          size = 0;
        bool             frozen = false; // The buffer contents are immutable.
        bool             shared = false; // Reference counted (not unique).
        bool             loaned = false; // Globally tracked.
        uint64_t         pinned_epoch;
        BufferDescriptor descriptor;
    };

}
