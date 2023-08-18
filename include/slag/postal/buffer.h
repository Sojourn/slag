#pragma once

#include <span>
#include <compare>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag::postal {

#if 0
    struct BufferDescriptor {
        uint32_t offset;
        uint16_t region;

        struct {
            bool shared;
            bool frozen;
        } flags;

        constexpr auto operator<=>(const BufferDescriptor&) const = default;
    };
#endif

    struct BufferDescriptor {
        uint32_t index  : 30; // TODO: calculate the region from this
        uint32_t shared :  1;
        uint32_t global :  1;

        BufferDescriptor();

        constexpr auto operator<=>(const BufferDescriptor&) const = default;

        constexpr explicit operator bool() const {
            return index != 0;
        }
    };

    class BufferHandle {
    private:
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

        BufferDescriptor descriptor() const;

        // Returns a new handle to a copy of the data.
        BufferHandle clone();

        // Returns a new handle to the same underlying data.
        // Falls back to clone if the underlying data isn't frozen.
        BufferHandle share();

        // Releases our reference to the underlying data.
        void reset();

    private:
        BufferDescriptor descriptor_;
    };

}
