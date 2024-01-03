#pragma once

#include <array>
#include <vector>
#include "slag/core/resource.h"
#include "slag/collections/queue.h"

namespace slag {

    class ResourceTable {
    public:
        Queue<ResourceBase*>& garbage();

        ResourceBase* select(ResourceDescriptor descriptor);

        void insert(ResourceBase& resource);

        void increment_reference_count(ResourceBase& resource);
        void decrement_reference_count(ResourceBase& resource);

    private:
        struct Record {
            ResourceBase* resource = nullptr;
            uint32_t      reference_count = 0;
        };

        void increment_reference_count(Record& record);
        void decrement_reference_count(Record& record);

        Record& get_record(ResourceDescriptor descriptor);

    private:
        using Partition = std::vector<Record>;

        std::vector<Partition> partitions_;
        Queue<ResourceBase*>   garbage_;
    };

    using ResourceTables = std::array<ResourceTable, RESOURCE_TYPE_COUNT>;

}
