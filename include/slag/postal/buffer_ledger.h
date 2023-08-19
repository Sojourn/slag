#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "slag/queue.h"
#include "slag/pool_allocator.h"
#include "slag/postal/types.h"
#include "slag/postal/config.h"
#include "slag/postal/domain.h"
#include "slag/postal/buffer.h"
#include "slag/postal/census.h"

namespace slag::postal {

    class BufferAllocator {
    public:
        virtual ~BufferAllocator() = default;

        virtual BufferDescriptor allocate_buffer(size_t size) = 0;
        virtual void reallocate_buffer(BufferDescriptor descriptor, size_t size) = 0;
        virtual void deallocate_buffer(BufferDescriptor descriptor) = 0;
    };

    struct BufferSegment {
        std::span<std::byte> data;
        BufferSegment*       next;
    };

    template<DomainType domain_type>
    struct BufferLedgerEntry;

    template<DomainType domain_type>
    struct BufferLedgerEntry<DomainType::NATION> {
        BufferSegment    head;
        BufferSegment*   tail;       // For efficient extension.
        uint32_t         size;       // Combined size of all segments.

        uint16_t         region;
        uint16_t         pinned : 1; // Memory layout will not change.
        uint16_t         frozen : 1; // Memory contents will not change.

        BufferAllocator* allocator;
    };

    // This should be kept small, as we need a per-region instance.
    template<DomainType domain_type>
    struct BufferLedgerEntry<DomainType::REGION> {
        uint8_t reference_count;
    };

    template<DomainType domain_type>
    class BufferLedger;

    template<>
    class BufferLedger<DomainType::NATION> {
    public:
        using Entry = BufferLedgerEntry<DomainType::NATION>;

        explicit BufferLedger(const DomainConfig<DomainType::NATION>& config);

    private:
        std::vector<Entry> entries_;
    };

    template<>
    class BufferLedger<DomainType::REGION> {
    public:
        using Entry = BufferLedgerEntry<DomainType::REGION>;

        BufferLedger(BufferLedger<DomainType::NATION>& national_ledger, const DomainConfig<DomainType::REGION>& config);

        void set_frozen();
        bool is_frozen() const;

        void set_pinned();
        bool is_pinned() const;

    private:
        friend class BufferHandle;

        void increment_reference_count(BufferDescriptor descriptor);
        void decrement_reference_count(BufferDescriptor descriptor);

    private:
        friend class BufferAllocator;

        BufferDescriptor allocate_descriptor();
        void deallocate_descriptor(BufferDescriptor descriptor);

        BufferSegment allocate_segment();
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

        BufferLedger<DomainType::NATION>& national_ledger_;
        std::vector<Entry>                entries_;
        std::vector<uint32_t>             unused_entries_;
        std::vector<PendingExportQueue>   pending_export_queues_;
        PoolAllocator<BufferSegment>      segment_allocator_;
    };

}
