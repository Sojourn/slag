#pragma once

#include <cstdint>
#include <cstddef>

// TIMER
// FILE/SOCKET
#define SLAG_RESOURCE_TYPES(X) \
    X(BUFFER)                  \
    X(OPERATION)               \

namespace slag {

        enum class ResourceType : uint8_t {
    #define X(SLAG_RESOURCE_TYPE) \
            SLAG_RESOURCE_TYPE,   \

            SLAG_RESOURCE_TYPES(X)
    #undef X
        };

        constexpr size_t RESOURCE_TYPE_COUNT = 0
    #define X(SLAG_RESOURCE_TYPE) \
            + 1                   \

            SLAG_RESOURCE_TYPES(X)
    #undef X
        ;

        constexpr size_t to_index(ResourceType type) {
            return static_cast<size_t>(type);
        }

}
