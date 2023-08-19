#pragma once

#include <span>
#include <compare>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag::postal {

    struct BufferDescriptor {
        uint32_t index;
    };

    class BufferHandle {
    public:
        BufferHandle();
        explicit BufferHandle(BufferDescriptor descriptor);
        ~BufferHandle();

        BufferHandle(BufferHandle&& other);
        BufferHandle(const BufferHandle&) = delete;
        BufferHandle& operator=(BufferHandle&& that);
        BufferHandle& operator=(const BufferHandle&) = delete;

        explicit operator bool() const;

        BufferDescriptor descriptor() const;

        // Returns a handle to a copy of this buffer's data.
        BufferHandle clone() const;

        // Releases our reference to the underlying data.
        void reset();

    private:
        BufferDescriptor descriptor_;
    };

}
