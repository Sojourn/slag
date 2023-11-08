#pragma once

#include <array>
#include <bitset>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/types.h"
#include "slag/config.h"
#include "slag/collection/bit_set.h"
#include "slag/collection/queue.h"

namespace slag {

    // Clone current model.
    // Apply deltas to clone.
    // Scan the old model for buffers that were being used +
    //   see if they are no longer being used in the new epoch. // // Potential issues to think about:
    //   probably need to keep an actual counter, along with
    //   attach/detach bitsets instead of a toggling.
    // Somewhat complex logic for merging reference counts while keeping
    // information density high. Consider allocating a share/counter struct
    // and keeping a pointer to it in the description.

    // per-thread/buffer bitset + toggle will work
    // support up to 64-threads per process + we can use that as a mask

    // 64 * 64 * (1 << 16) == 256MB for a fully connected network of threads
    // thread clusters -> process

    // A summary of buffers referenced by a thread at a particular moment in time.
    //
    // BufferReferenceDelta?
    // BufferReferenceSnapshot? // Not really a 'snapshot' since it isn't two-sided.
    //
    class BufferCensus {
    public:
        using Partition = BitSet;

        std::span<const Partition> partitions() const {
            return {
                partitions_.data(),
                partitions_.size(),
            };
        }

        bool test(BufferDescriptor descriptor) const {
            assert(descriptor.thread_index < INVALID_THREAD_INDEX);
            assert(descriptor.buffer_index < INVALID_BUFFER_INDEX);

            const Partition& partition = partitions_[descriptor.thread_index];
            if (partition.size_bits() <= descriptor.buffer_index) {
                return false;
            }

            return partition.test(descriptor.buffer_index);
        }

        void set(BufferDescriptor descriptor) {
            assert(descriptor.thread_index < INVALID_THREAD_INDEX);
            assert(descriptor.buffer_index < INVALID_BUFFER_INDEX);

            reserve(descriptor);

            partitions_[thread_index].set(buffer_index);
        }

        void flip(BufferDescriptor descriptor) {
            assert(descriptor.thread_index < INVALID_THREAD_INDEX);
            assert(descriptor.buffer_index < INVALID_BUFFER_INDEX);

            reserve(descriptor);

            partitions_[thread_index].flip(buffer_index);
        }

        void flip(const BufferCensus& other) {
            for (ThreadIndex thread_index = 0; thread_index < MAX_THREAD_COUNT; ++thread_index) {
                const Partition& other_partition = other.partitions()[thread_index];

                BitSetScanner other_partition_scanner(other_partition);
                while (std::optional<size_t> buffer_index = other_partition_scanner.next()) {
                    BufferDescriptor descriptor = {
                        .buffer_index = *buffer_index,
                        .thread_index = thread_index,
                    };

                    flip(descriptor);
                }
            }
        }

        void reset(BufferDescriptor descriptor) {
            assert(descriptor.thread_index < INVALID_THREAD_INDEX);
            assert(descriptor.buffer_index < INVALID_BUFFER_INDEX);

            reserve(descriptor);

            partitions_[thread_index].reset(buffer_index);
        }

        void reset() {
            for (Partition& partition: partitions_) {
                partition.reset();
            }
        }

    private:
        void reserve(BufferDescriptor descriptor) {
            Partition& partition = partitions_[thread_index];
            if (remote_references_partition.size_bits() <= buffer_index) {
                remote_references_partition.grow_size_bits(buffer_index + 1);
            }
        }

    private:
        std::array<Partition, MAX_THREAD_COUNT> partitions_;
    };

    // TODO: Parallel primitives like parallel-for.
    //       Think about using epoll in single-shot mode to recruit random sleeping threads.
    //       Busy threads are unlikely to 'win' the work.
    //       Can 'isolate' threads by not looking at the global job queue. Can have multiple
    //       job queues to 'isolate' workloads differently.
    //       This form of threading should maintain causality.
    //       System threads for use by the runtime?
    //

    // TODO: Call this the GarbageCollector and make it fancy to support multithreaded collections.
    // TODO: Make some functions implementing the GC algo templated so they can be reused (use atomics or not).
    // TODO: Make this an interface so we can choose a single/multi-threaded collector implementation.
    class GarbageCollector {
        using Partition = std::vector<std::bitset<MAX_THREAD_COUNT>>;
        using Table = std::vector<Partition>;

    public:
        // TODO: think about making this non-blocking w/ tasks.
        void mark_and_sweep(std::span<const BufferCensus> censuses) {
            advance_table_version();

            parallel_mark();
            parallel_sweep();
        }

        void parallel_mark(std::span<const BufferCensus> censuses) {
            // TODO: parallel-for (w/ atomic bitset operations).
            for (ThreadIndex thread_index = 0; thread_index < censuses.size(); ++thread_index) {
                mark(thread_index, census);
            }
        }

        // This should return an array of the highest buffer index we toggled for each partition.
        // Then we can bail when process_censuses reaches the index instead of scanning everything.
        //
        // There are ways of turning this from O(dead+alive) to O(changes) which should be ~O(dead) (in steady state).
        // Put the buffer index in a vector to the side + merge them all to let us skip rows. Still need to scan the
        // bitsets though.
        //
        void mark(ThreadIndex thread_index, const BufferCensus& census) {
            Table& table = current_table_version();

            std::span<const BufferCensus::Partition> census_partitions = delta.partitions();
            for (size_t partition_index = 0; partition_index < census_partitions.size(); ++partition_index) {
                const BufferCensus::Partition& census_partition = census_partitions[partition_index];

                BitSetScanner scanner(census_partition);
                while (std::optional<size_t> buffer_index = scanner.next()) {
                    mark_descriptor(table, thread_index, BufferDescriptor {
                        .buffer_index = *buffer_index,
                        .thread_index = static_cast<ThreadIndex>(partition_index),
                    });
                }
            }
        }

        void parallel_sweep() {
            // TODO: parallel-for.
            for (ThreadIndex thread_index = 0; thread_index < censuses.size(); ++thread_index) {
                sweep(thread_index);
            }
        }

    private:
        Table& current_table_version() {
            return table_versions_[epoch & 1];
        }

        Table& previous_table_version() {
            return table_versions_[(epoch - 1) & 1];
        }

        void advance_table_version() {
            version_ += 1;

            current_table_version() = previous_table_version();
        }

        // Flip the bit for this thread corresponding to the descriptor's row in the table.
        void mark(Table& table, ThreadIndex thread_index, BufferDescriptor descriptor) {
            Partition& partition = table[descriptor.thread_index];

            // Ensure the partition contains a row for this descriptor.
            if (partition.size() <= descriptor.buffer_index) {
                size_t new_partition_size = 1;
                while (new_partition_size < descriptor.buffer_index) {
                    new_partition_size *= 2;
                }

                partition.resize(new_partition_size);
            }

            partition[descriptor.buffer_index].flip(thread_index);
        }

        void sweep(ThreadIndex thread_index) {
        }

    private:
        uint64_t version_;
        std::array<Table, 2> table_versions_;
        std::array<std::vector<BufferDescriptor>, MAX_THREAD_COUNT> garbage_;
    };

}
