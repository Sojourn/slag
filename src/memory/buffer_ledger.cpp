#include "slag/memory/buffer_ledger.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    BufferLedger::BufferLedger(int16_t worker_index)
        : worker_index_{worker_index}
    {
    }

    BufferHandle BufferLedger::create_buffer() {
        BufferDescription& description = description_allocator_.allocate();
        description.descriptor = {
            .buffer_index = buffer_index,
            .worker_index = worker_index_,
        };

        return {
            description
        };
    }

    BufferHandle BufferLedger::import_buffer(BufferDescription& remote_description) {
        assert(remote_description.descriptor.worker_index != worker_index_);
        assert(remote_description.frozen);
        assert(remote_description.shared);

        initialize_reference_counter(remote_description);
        increment_reference_count(remote_description);

        return {
            description
        };
    }

    void BufferLedger::set_buffer_frozen(BufferDescription& description) {
        if (description.frozen) {
            return;
        }

        description.frozen = true;
    }

    void BufferLedger::set_buffer_shared(BufferDescription& description) {
        if (description.shared) {
            return;
        }

        description.shared = true;

        initialize_reference_counter(description);
        increment_reference_count(description);
    }

    void BufferLedger::append_buffer_storage(BufferDescription& description, BufferStorage storage) {
        assert(description.descriptor.worker_index == worker_index_);
        assert(!description.frozen);
        assert(storage.capacity >= storage.size);
        assert(storage.size);

        if (!storage.capacity) {
            return;
        }

        BufferSegment* segment;
        if (description.tail) {
            segment = &segment_allocator_.allocate();
            description.tail->next = segment;
        }
        else {
            segment = &description.head;
        }

        segment->storage = storage;
        segment->size += storage.size;
    }

    Event& BufferLedger::readable_event() {
        return garbage_event_;
    }

    size_t BufferLedger::reap_buffer_storage(std::span<BufferStorage> storage_array) {
        size_t garbage_count = garbage_.size();
        size_t count = std::min(storage_array.size(), garbage_.size());

        // Move from our garbage stack into the out parameter in LIFO order.
        for (size_t i = 0; i < count; ++i) {
            storage_array[i] = garbage_[garbage_count - i - 1];
        }

        // Update internal state.
        garbage_.resize(garbage_.size() - count);
        if (garbage_.empty()) {
            garbage_event_.reset();
        }

        return count;
    }

    void BufferLedger::initialize_reference_counter(BufferDescription& description) {
        size_t buffer_index = static_cast<size_t>(description.descriptor.buffer_index);
        size_t worker_index = static_cast<size_t>(description.descriptor.worker_index);

        // Ensure we have a partition for this worker.
        if (reference_counts_.size() <= worker_index) {
            reference_counts_.resize(worker_index + 1);
        }

        // Ensure the partition contains a reference counter for this buffer.
        auto&& partition = reference_counts_[worker_index];
        if (partition.size() <= buffer_index) {
            size_t partition_size = 1;
            while (partition_size <= buffer_index) {
                partition_size *= 2;
            }

            partition.resize(partition_size);
        }
    }

    void BufferLedger::increment_reference_count(BufferDescription& description) {
        size_t buffer_index = static_cast<size_t>(description.descriptor.buffer_index);
        size_t worker_index = static_cast<size_t>(description.descriptor.worker_index);

        auto&& reference_counter = reference_counts_[worker_index][buffer_index];
        reference_counter += 1;
    }

    void BufferLedger::decrement_reference_count(BufferDescription& description) {
        size_t buffer_index = static_cast<size_t>(description.descriptor.buffer_index);
        size_t worker_index = static_cast<size_t>(description.descriptor.worker_index);

        // Decrement the local reference count if the buffer has been shared.
        auto&& reference_counter = reference_counts_[worker_index][buffer_index];
        if (description.shared) {
            reference_counter -= 1;
        }

        // Deallocate the buffer once the reference count drops to zero.
        if (reference_counter == 0) {
            deallocate_buffer(description);
        }
    }

    void BufferLedger::deallocate_buffer(BufferDescription& description) {
        if (description.descriptor.worker_index == worker_index) {
            deallocate_local_buffer(description);
        }
        else {
            deallocate_foreign_buffer(description);
        }
    }

    // This can be O(1) by splicing segment lists, but would require an allocation.
    void BufferLedger::deallocate_local_buffer(BufferDescription& description) {
        BufferSegment* segment = &description.head;

        // The first segment is not allocated and needs special treatment.
        deallocate_buffer_storage(segment->storage);
        segment = segment->next;

        // Deallocate remaining, dynamic segments.
        while (segment) {
            segment = deallocate_buffer_segment(*segment);
        }

        deallocate_buffer_index(description.descriptor.buffer_index);
        description_allocator_.deallocate(description);
    }

    void BufferLedger::deallocate_foreign_buffer(BufferDescription& description) {
        // TODO: update the census.
    }

    BufferSegment* BufferLedger::deallocate_buffer_segment(BufferSegment& segment) {
        BufferSegment* next = segment.next; // Stash the next pointer.

        deallocate_buffer_segment(segment.storage);
        segment_allocator_.deallocate(segment);

        return next;
    }

    void BufferLedger::deallocate_buffer_storage(BufferStorage& storage) {
        if (storage.capacity == 0) {
            return; // The segment containing this was not initialized.
        }

        garbage_.push_back(storage);
    }

    int32_t BufferLedger::allocate_buffer_index() {
        if (buffer_index_allocator_.empty()) {
            return next_buffer_index_++;
        }

        // Reuse an old buffer index.
        int32_t buffer_index = buffer_index_allocator_.back();
        buffer_index_allocator_.pop_back();
        return buffer_index;
    }

    void BufferLedger::deallocate_buffer_index(int32_t buffer_index) {
        buffer_index_allocator_.push_back(buffer_index);
    }

}
