#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag::postal {

    class BufferTable;

    // TODO: rename the header/source files to match the class name
    class BufferHandle {
    private:
        friend class BufferTable;
        friend class BufferCustodian;

        explicit BufferHandle(BufferDescriptor descriptor);

    public:
        BufferHandle() = default;
        ~BufferHandle();

        BufferHandle(BufferHandle&& other);
        BufferHandle(const BufferHandle&) = delete;
        BufferHandle& operator=(BufferHandle&& rhs);
        BufferHandle& operator=(const BufferHandle&) = delete;

        explicit operator bool() const;

        bool is_valid() const;
        bool is_shared() const;
        bool is_global() const;
        bool is_frozen() const;

        const BufferIdentity& identity() const;
        const BufferProperties& properties() const;
        const BufferDescriptor& descriptor() const;

        // Returns a new handle to a copy of the data.
        BufferHandle clone();

        // Returns a new handle to the same underlying data.
        // Falls back to clone if the underlying data isn't frozen.
        BufferHandle share();

        // Releases our reference to the underlying data.
        void reset();

    private:
        BufferTable& table();

    private:
        BufferDescriptor descriptor_;
    };

    uint16_t to_scaled_capacity(size_t capacity);
    size_t from_scaled_capacity(uint16_t scaled_capacity);

}
