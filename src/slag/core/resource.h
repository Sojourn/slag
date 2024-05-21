#pragma once

#include "mantle/mantle.h"
#include "resource_types.h"

namespace slag {

    template<ResourceType type>
    class Resource;

#define X(SLAG_RESOURCE_TYPE)                         \
    template<>                                        \
    class Resource<ResourceType::SLAG_RESOURCE_TYPE>; \

#undef X

    class ResourceBase : public mantle::Object {
    protected:
        explicit ResourceBase(ResourceType type);

    public:
        ~ResourceBase() = default;

        ResourceBase(ResourceBase&&) = delete;
        ResourceBase(const ResourceBase&) = delete;
        ResourceBase& operator=(ResourceBase&&) = delete;
        ResourceBase& operator=(const ResourceBase&) = delete;

        [[nodiscard]] ResourceType resource_type() const;
    };

    template<typename ResourceVisitor>
    void visit(ResourceVisitor&& visitor, ResourceBase& resource);

}

#include "resource.hpp"
