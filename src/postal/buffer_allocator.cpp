#include "slag/postal/buffer_allocator.h"
#include "slag/postal/buffer_ledger.h"
#include "slag/core/domain.h"
#include <limits>
#include <stdexcept>
#include <cassert>

namespace slag {

    static constexpr size_t MAX_BUFFER_SEGMENT_COUNT = std::numeric_limits<
        decltype(NationalBufferLedgerEntry::segment_count)
    >::max();

    inline std::span<std::byte> get_buffer_segment_storage(BufferSegment& segment) {
        return {
            reinterpret_cast<std::byte*>(segment.data),
            segment.capacity
        };
    }

    inline void set_buffer_segment_storage(BufferSegment& segment, std::span<std::byte> storage) {
        segment.data = storage.data();
        segment.capacity = storage.size_bytes();
    }

    BufferAllocator::BufferAllocator(Region& region)
        : region_{region}
        , regional_buffer_ledger_{region.buffer_ledger()}
    {
    }

    NationalBufferLedger& BufferAllocator::national_buffer_ledger() {
        return regional_buffer_ledger_.national_buffer_ledger();
    }

    RegionalBufferLedger& BufferAllocator::regional_buffer_ledger() {
        return regional_buffer_ledger_;
    }

    BufferDescriptor BufferAllocator::allocate_buffer() {
        BufferDescriptor descriptor = regional_buffer_ledger_.allocate_descriptor();

        // Initialize the regional entry.
        {
            auto&& entry = regional_buffer_ledger_.get_regional_entry(descriptor);

            entry.reference_count = 0;
        }

        // Initialize the national entry.
        {
            auto&& entry = regional_buffer_ledger_.get_national_entry(descriptor);

            entry.head.next     = nullptr;
            entry.head.data     = nullptr;
            entry.head.capacity = 0;
            entry.tail          = nullptr;
            entry.size          = 0;
            entry.segment_count = 0;
            entry.region        = region_.post_area().region;
            entry.pinned        = 0;
            entry.frozen        = 0;
            entry.allocator     = this;
        }

        return descriptor;
    }

    void BufferAllocator::deallocate_buffer(BufferDescriptor descriptor) {
        NationalBufferLedgerEntry& national_entry = regional_buffer_ledger_.get_national_entry(descriptor);

        BufferSegment* segment = &national_entry.head;

        for (uint8_t i = 0; i < national_entry.segment_count; ++i) {
            BufferSegment* next_segment = segment->next;

            deallocate_segment_storage(get_buffer_segment_storage(*segment));
            if (i > 0) {
                regional_buffer_ledger_.deallocate_segment(*segment);
            }

            segment = next_segment;
        }
    }

    std::span<std::byte> BufferAllocator::allocate_buffer_segment(BufferDescriptor descriptor, size_t size_hint) {
        NationalBufferLedgerEntry& national_entry = regional_buffer_ledger_.get_national_entry(descriptor);

        if (size_hint == 0) {
            assert(false);
            return {};
        }

        // Allocate the segment.
        BufferSegment* segment = nullptr;
        if (national_entry.segment_count) {
            if (national_entry.segment_count == MAX_BUFFER_SEGMENT_COUNT) {
                throw std::runtime_error("Too many buffer segments");
            }

            segment = &regional_buffer_ledger_.allocate_segment();
            national_entry.tail->next = segment;
        }
        else {
            segment = &national_entry.head;
            assert(!national_entry.tail);
        }

        segment->next = nullptr;
        national_entry.tail = segment;
        national_entry.segment_count += 1;

        // Allocate storage for the segment.
        std::span<std::byte> segment_storage = allocate_segment_storage(size_hint);
        set_buffer_segment_storage(*segment, segment_storage);
        return segment_storage;
    }

}
