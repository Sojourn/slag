#include "slag/postal/buffer_ledger.h"
#include "slag/postal/domain.h"
#include <limits>
#include <stdexcept>
#include <cstring>
#include <cassert>

namespace slag::postal {

    NationalBufferLedger::NationalBufferLedger(Nation& nation)
        : nation_{nation}
    {
        auto&& national_config = nation_.config();

        entries_.resize(national_config.buffer_count);
    }

    auto NationalBufferLedger::get_national_entry(BufferDescriptor descriptor) -> NationalEntry& {
        return entries_[descriptor.index];
    }

    RegionalBufferLedger::RegionalBufferLedger(Region& region)
        : region_{region}
        , regional_census_{*region.census_cursor()}
        , national_buffer_ledger_{region.nation().buffer_ledger()}
    {
        auto&& national_config = region_.nation().config();
        auto&& regional_config = region_.config();

        entries_.resize(national_config.buffer_count);
        pending_export_queues_.resize(national_config.region_count);

        {
            auto&& [first, last] = regional_config.buffer_range;

            unused_entries_.reserve(last - first);
            for (size_t i = first; i < last; ++i) {
                unused_entries_.push_back(i);
            }
        }
    }

    NationalBufferLedger& RegionalBufferLedger::national_buffer_ledger() {
        return national_buffer_ledger_;
    }

    auto RegionalBufferLedger::get_national_entry(BufferDescriptor descriptor) -> NationalEntry& {
        return national_buffer_ledger_.get_national_entry(descriptor);
    }

    auto RegionalBufferLedger::get_regional_entry(BufferDescriptor descriptor) -> RegionalEntry& {
        return entries_[descriptor.index];
    }

    void RegionalBufferLedger::increment_reference_count(BufferDescriptor descriptor) {
        auto&& regional_entry = get_regional_entry(descriptor);

        if (regional_entry.reference_count == std::numeric_limits<decltype(RegionalBufferLedgerEntry::reference_count)>::max()) {
            throw std::runtime_error("To many handles to buffer");
        }

        regional_entry.reference_count += 1;
    }

    void RegionalBufferLedger::decrement_reference_count(BufferDescriptor descriptor) {
        auto&& regional_entry = get_regional_entry(descriptor);

        // Check if the local reference count has dropped below zero.
        if (regional_entry.reference_count == 0) {
            auto&& national_entry = get_national_entry(descriptor);

            if (national_entry.region == region_.post_area().region) {
                deallocate_descriptor(descriptor);
            }
            else {
                auto&& buffer_reference_changes = regional_census_->buffer_reference_changes;
                buffer_reference_changes.flip(descriptor.index);
            }
        }
        else {
            regional_entry.reference_count -= 1;
        }
    }

    BufferDescriptor RegionalBufferLedger::allocate_descriptor() {
        if (unused_entries_.empty()) {
            throw std::runtime_error("Failed to allocate buffer descriptor");
        }

        uint32_t index = unused_entries_.back();
        unused_entries_.pop_back();

        return {
            .index = index,
        };
    }

    void RegionalBufferLedger::deallocate_descriptor(BufferDescriptor descriptor) {
        NationalEntry& national_entry = get_national_entry(descriptor);
        national_entry.allocator->deallocate_buffer(descriptor);
        unused_entries_.push_back(descriptor.index);
    }

    BufferSegment& RegionalBufferLedger::allocate_segment() {
        BufferSegment& segment = segment_allocator_.allocate();
        memset(&segment, 0, sizeof(segment));
        return segment;
    }

    void RegionalBufferLedger::deallocate_segment(BufferSegment& segment) {
        segment_allocator_.deallocate(segment);
    }

}
