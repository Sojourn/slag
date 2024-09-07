#pragma once

#include "slag/object.h"

#include <cstdint>

#define SLAG_RESOURCE_TYPES(X) \
    X(MANAGED)                 \
    X(MESSAGE)                 \
    X(BUFFER)                  \
    X(FILE_DESCRIPTOR)         \
    X(OPERATION)               \

namespace slag {

    enum class ResourceType : uint8_t {
#define X(SLAG_RESOURCE_TYPE) \
        SLAG_RESOURCE_TYPE,   \

        SLAG_RESOURCE_TYPES(X)
#undef X
    };

    template<ResourceType type>
    class Resource;

    template<>
    class Resource<ResourceType::MANAGED> : public Object {
    public:
        Resource()
            : Object(static_cast<ObjectGroup>(ResourceType::MANAGED))
        {
        }
    };

    using Managed        = Resource<ResourceType::MANAGED>;
    using Message        = Resource<ResourceType::MESSAGE>;
    using Buffer         = Resource<ResourceType::BUFFER>;
    using FileDescriptor = Resource<ResourceType::FILE_DESCRIPTOR>;
    using Operation      = Resource<ResourceType::OPERATION>;

}
