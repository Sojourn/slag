#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/postal/types.h"
#include "slag/postal/buffer.h"

namespace slag {

    // System: uses the c.r.t. allocator.
    // Linear: linear allocation 
    // Cohort: ~fifo allocations.

    class Nation;
    class Region;

    class NationalBufferLedger;
    class RegionalBufferLedger;

    // NOTE: different implementations can choose different alignments,
    //       all the way down to single byte alignment (none).
    //
    class BufferAllocator {
        BufferAllocator(BufferAllocator&&) = delete;
        BufferAllocator(const BufferAllocator&) = delete;
        BufferAllocator& operator=(BufferAllocator&&) = delete;
        BufferAllocator& operator=(const BufferAllocator&) = delete;

    public:
        explicit BufferAllocator(Region& region);
        virtual ~BufferAllocator() = default;

        NationalBufferLedger& national_buffer_ledger();
        RegionalBufferLedger& regional_buffer_ledger();

        BufferDescriptor allocate_buffer();
        void deallocate_buffer(BufferDescriptor descriptor);
        std::span<std::byte> allocate_buffer_segment(BufferDescriptor descriptor, size_t size_hint);

    private:
        virtual std::span<std::byte> allocate_segment_storage(size_t size_hint) = 0;
        virtual void deallocate_segment_storage(std::span<std::byte> storage) = 0;

    private:
        Region&               region_;
        RegionalBufferLedger& regional_buffer_ledger_;
    };

}
