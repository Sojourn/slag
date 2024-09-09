#pragma once

#include "slag/object.h"

#include <cstdint>

#define SLAG_LIB_RESOURCE_TYPES(X) \
    X(MANAGED)                     \
    X(MESSAGE)                     \
    X(BUFFER)                      \
    X(FILE_DESCRIPTOR)             \
    X(OPERATION)                   \

// Resource types can be injected by defining this.
// TODO: Think about how to finalize these.
#ifndef SLAG_APP_RESOURCE_TYPES
#  define SLAG_APP_RESOURCE_TYPES(X)
#endif

#define SLAG_RESOURCE_TYPES(X) \
    SLAG_LIB_RESOURCE_TYPES(X) \
    SLAG_APP_RESOURCE_TYPES(X) \

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

    constexpr bool is_lib_resource(ResourceType type) {
        (void)type;

#define X(SLAG_LIB_RESOURCE_TYPE)                                      \
        if (type == ResourceType::SLAG_LIB_RESOURCE_TYPE) return true; \

        SLAG_LIB_RESOURCE_TYPES(X)
#undef X

        return false;
    }

    constexpr bool is_app_resource(ResourceType type) {
        (void)type;

#define X(SLAG_APP_RESOURCE_TYPE)                                      \
        if (type == ResourceType::SLAG_APP_RESOURCE_TYPE) return true; \

        SLAG_APP_RESOURCE_TYPES(X)
#undef X

        return false;
    }

}
