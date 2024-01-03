#pragma once

#include <compare>
#include <cstdint>
#include <cstddef>
#include "slag/types.h"
#include "slag/collections/intrusive_queue.h"
#include "resource_types.h"

namespace slag {

    using ResourceRecordIndex = uint32_t;

    struct ResourceDescriptor {
        ThreadIndex         owner;
        ResourceType        type;
        ResourceRecordIndex index;

        auto operator<=>(const ResourceDescriptor&) const = default;
    };

    class ResourceBase {
    public:
        explicit ResourceBase(ResourceDescriptor descriptor);
        ~ResourceBase() = default;

        ResourceBase(ResourceBase&&) = delete;
        ResourceBase(const ResourceBase&) = delete;
        ResourceBase& operator=(ResourceBase&&) = delete;
        ResourceBase& operator=(const ResourceBase&) = delete;

        ResourceType type() const;
        ResourceDescriptor descriptor() const;

    private:
        ResourceDescriptor descriptor_;
    };

    template<ResourceType type>
    class Resource;

}

#include "resource.hpp"
