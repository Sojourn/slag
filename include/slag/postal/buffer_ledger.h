#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "slag/queue.h"
#include "slag/pool_allocator.h"
#include "slag/postal/types.h"
#include "slag/postal/config.h"
#include "slag/postal/census.h"
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_allocator.h"

namespace slag::postal {

    class NationalBufferLedger;
    class RegionalBufferLedger;

    // TODO: think about making this a variant
    struct BufferSegment {
        BufferSegment* next;

        // variant of BufferHandle + span to allow splicing?
        // reader would need to maintain a handle stack
        void*          data;
        size_t         capacity;
    };

    struct NationalBufferLedgerEntry {
        BufferSegment    head;
        BufferSegment*   tail;          // For efficient extension.
        size_t           size;          // Potentially != total capacity of segments.
        uint8_t          segment_count; // Useful for sizing iovecs.
        uint8_t          pinned : 1;    // Memory layout will not change.
        uint8_t          frozen : 1;    // Memory contents will not change.
        uint16_t         region;
        BufferAllocator* allocator;
    };

    struct RegionalBufferLedgerEntry {
        // NOTE: the reference count is intentionally off-by-one. The buffer is not
        //       considered to be unreferenced until it would drop below zero.
        //       This lets us elide reference counting entirely for uniquely owned buffers.
        //
        uint8_t reference_count;
    };

    class NationalBufferLedger {
    public:
        using NationalEntry = NationalBufferLedgerEntry;

        explicit NationalBufferLedger(Nation& nation);

        NationalEntry& get_national_entry(BufferDescriptor descriptor);

    private:
        Nation&                    nation_;
        std::vector<NationalEntry> entries_;
    };

    class RegionalBufferLedger {
    public:
        using NationalEntry = NationalBufferLedgerEntry;
        using RegionalEntry = RegionalBufferLedgerEntry;

        explicit RegionalBufferLedger(Region& region);

        NationalBufferLedger& national_buffer_ledger();

        NationalEntry& get_national_entry(BufferDescriptor descriptor);
        RegionalEntry& get_regional_entry(BufferDescriptor descriptor);

    private:
        friend class BufferHandle;

        void increment_reference_count(BufferDescriptor descriptor);
        void decrement_reference_count(BufferDescriptor descriptor);

    private:
        friend class BufferAllocator;

        BufferDescriptor allocate_descriptor();
        void deallocate_descriptor(BufferDescriptor descriptor);

        BufferSegment& allocate_segment();
        void deallocate_segment(BufferSegment& segment);

    private:
        // Keep a buffer alive until it has been confirmed by the region.
        void protect_regional_delivery(uint16_t region, BufferHandle buffer_handle);

        // Deliveries to this region have been confirmed up to this sequence. Remove protection for them.
       void confirm_regional_deliveries(uint16_t region, uint64_t sequence);

    private:
        struct PendingExportQueue {
            Queue<BufferHandle> buffers;
            uint64_t            confirmed_sequence = 0;
        };

        Region&                            region_;
        DomainCensus<DomainType::REGION>*& regional_census_;
        NationalBufferLedger&              national_buffer_ledger_;
        std::vector<RegionalEntry>         entries_;
        std::vector<uint32_t>              unused_entries_;
        std::vector<PendingExportQueue>    pending_export_queues_;
        PoolAllocator<BufferSegment>       segment_allocator_;
    };

}
