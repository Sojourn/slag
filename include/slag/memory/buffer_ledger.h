#pragma once

#include <span>
#include <array>
#include <vector>
#include "slag/bit_set.h"
#include "slag/pool_allocator.h"
#include "slag/core/event.h"
#include "slag/memory/buffer_types.h"
#include "slag/memory/buffer_handle.h"

namespace slag {

    class BufferLedger : public Pollable<PollableType::READABLE> {
    public:
        BufferLedger(int16_t worker_index);

        BufferHandle create_buffer();
        BufferHandle import_buffer(BufferDescription& remote_description);

        void set_buffer_frozen(BufferDescription& description);
        void set_buffer_shared(BufferDescription& description);

        void append_buffer_storage(BufferDescription& description, BufferStorage storage);

    public:
        Event& readable_event() override final;

        size_t reap_buffer_storage(std::span<BufferStorage> storage_array);

    private:
        friend class BufferHandle;

        void initialize_reference_counter(BufferDescriptor descriptor);

        void increment_reference_count(BufferDescription& description);
        void decrement_reference_count(BufferDescription& description);

    private:
        void deallocate_buffer(BufferDescription& description);
        void deallocate_local_buffer(BufferDescription& description);
        void deallocate_remote_buffer(BufferDescription& description);

        BufferSegment* deallocate_buffer_segment(BufferSegment& segment);
        void deallocate_buffer_storage(BufferStorage& storage);

        int32_t allocate_buffer_index();
        void deallocate_buffer_index(int32_t buffer_index);

    private:
        using ReferenceCounterPartition = std::vector<uint8_t>;

        int16_t                                worker_index_;
        std::vector<ReferenceCounterPartition> reference_counts_;
        PoolAllocator<BufferDescription>       description_allocator_;
        PoolAllocator<BufferSegment>           segment_allocator_;

        // Idle task to keep this sorted? We can use std::partial_sort
        // to do it incrementally.
        std::vector<int32_t>                   buffer_index_allocator_;
        int32_t                                next_buffer_index_;

        std::vector<BufferStorage>             garbage_;
        Event                                  garbage_event_;
    };

}
