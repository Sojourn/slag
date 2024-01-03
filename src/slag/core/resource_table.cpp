#include "resource_table.h"
#include "slag/context.h"
#include <cstdlib>
#include <cassert>

namespace slag {

    Queue<ResourceBase*>& ResourceTable::garbage() {
        return garbage_;
    }

    ResourceBase* ResourceTable::select(ResourceDescriptor descriptor) {
        return get_record(descriptor).resource;
    }

    void ResourceTable::insert(ResourceBase& resource) {
        Record& record = get_record(resource.descriptor());

        if (record.resource) {
            increment_reference_count(record);
        }
        else {
            record.resource = &resource;
        }
    }

    void ResourceTable::increment_reference_count(ResourceBase& resource) {
        Record& record = get_record(resource.descriptor());
        assert(record.resource == &resource);
        increment_reference_count(record);

    }

    void ResourceTable::decrement_reference_count(ResourceBase& resource) {
        Record& record = get_record(resource.descriptor());
        assert(record.resource == &resource);
        decrement_reference_count(record);
    }

    void ResourceTable::increment_reference_count(Record& record) {
        record.reference_count += 1;
    }

    void ResourceTable::decrement_reference_count(Record& record) {
        if (record.reference_count) {
            record.reference_count -= 1;
        }
        else {
            garbage_.push_back(
                std::exchange(record.resource, nullptr)
            );
        }
    }

    auto ResourceTable::get_record(ResourceDescriptor descriptor) -> Record& {
        // Grow the number of partitions if needed. Should no-op in steady state.
        if (partitions_.size() <= descriptor.owner) {
            size_t new_partition_count = 1;
            while (new_partition_count < descriptor.owner) {
                new_partition_count *= 2;
            }

            partitions_.resize(new_partition_count);
        }

        // Grow the partition if needed. Should no-op in steady state.
        Partition& partition = partitions_[descriptor.owner];
        if (partition.size() <= descriptor.index) {
            partition.resize(descriptor.index);
            size_t new_partition_size = 1;
            while (new_partition_size < descriptor.index) {
                new_partition_size *= 2;
            }

            partition.resize(new_partition_size);
        }

        return partition[descriptor.index];
    }

}
