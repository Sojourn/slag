#pragma once

#include "slag/memory/buffer_descriptor.h"

namespace slag {

    // Stores a buffer descriptor and maintains the reference count for it.
    class BufferHandle {
        BufferHandle(BufferDescriptor descriptor);

    public:
        BufferHandle() = default;
        ~BufferHandle();

        BufferHandle(BufferHandle&& other);
        BufferHandle(const BufferHandle&) = delete;
        BufferHandle& operator=(BufferHandle&& that);
        BufferHandle& operator=(const BufferHandle&) = delete;

        explicit operator bool() const;

        BufferDescriptor descriptor() const;

        // Returns another handle to this buffer's data.
        BufferHandle share();

        // Releases our reference to the underlying data.
        void reset();

    private:
        void increment_reference_count();
        void decrement_reference_count();

    private:
        BufferDescriptor descriptor_;
    };

}
