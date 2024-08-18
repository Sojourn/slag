#pragma once

#include <cstdint>
#include "slag/core/object.h"

#define SLAG_RESOURCE_TYPES(X) \
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

    using Buffer         = Resource<ResourceType::BUFFER>;
    using FileDescriptor = Resource<ResourceType::FILE_DESCRIPTOR>;
    using Operation      = Resource<ResourceType::OPERATION>;

}
