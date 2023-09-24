#pragma once

#include <span>
#include <compare>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"

namespace slag {

    struct NationalBufferLedgerEntry;
    struct RegionalBufferLedgerEntry;

    struct BufferDescriptor {
        uint32_t index;
    };

    class BufferHandle {
        friend BufferHandle make_handle(BufferDescriptor descriptor);

        explicit BufferHandle(BufferDescriptor descriptor);

    public:
        BufferHandle();
        ~BufferHandle();

        BufferHandle(BufferHandle&& other);
        BufferHandle(const BufferHandle&) = delete;
        BufferHandle& operator=(BufferHandle&& that);
        BufferHandle& operator=(const BufferHandle&) = delete;

        explicit operator bool() const;

        // The descriptor is used as a key to tie everything together.
        BufferDescriptor descriptor() const;
        size_t size() const;

        // Representations of the underlying buffer are split by nation and region.
        NationalBufferLedgerEntry& national_entry();
        const NationalBufferLedgerEntry& national_entry() const;
        RegionalBufferLedgerEntry& regional_entry();
        const RegionalBufferLedgerEntry& regional_entry() const;

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

    // This should only be done once after allocating the buffer. Subsequent handles
    // should be created via BufferHandle::share, otherwise reference counts will
    // be messed up.
    //
    inline BufferHandle make_handle(BufferDescriptor descriptor) {
        return BufferHandle{descriptor};
    }

}
