#pragma once

#include <cstdint>
#include <cstddef>

#define SLAG_POSTAL_OPERATION_TYPES(X) \
    X(NOP)                             \
    X(OPEN)                            \
    X(CLOSE)                           \
    X(WRITE)                           \
    X(TIMER)                           \
    X(SOCKET)                          \
    X(CONNECT)                         \
    X(ACCEPT)                          \
    X(MADVISE)                         \
    X(INTERRUPT)                       \

namespace slag::postal {

    enum class OperationType : uint8_t {
#define X(SLAG_POSTAL_OPERATION_TYPE) \
        SLAG_POSTAL_OPERATION_TYPE,   \

        SLAG_POSTAL_OPERATION_TYPES(X)
#undef X
    };

    constexpr size_t OPERATION_TYPE_COUNT = 0
#define X(SLAG_POSTAL_OPERATION_TYPE) + 1
        SLAG_POSTAL_OPERATION_TYPES(X)
#undef X
    ;

    constexpr size_t to_index(OperationType operation_type) {
        return static_cast<size_t>(operation_type);
    }

}
