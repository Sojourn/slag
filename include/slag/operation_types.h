#pragma once

#include <cstddef>
#include <cstdint>

#define SLAG_OPERATION_TYPES(X) \
    X(NOP)                      \
    X(CANCEL)                   \
    X(ASSIGN)                   \
    X(CLOSE)                    \
    X(BIND)                     \
    X(CONNECT)                  \
    X(ACCEPT)                   \
    X(SEND)                     \
    X(RECEIVE)

namespace slag {

    enum class OperationType : uint8_t {
#define X(SLAG_OPERATION_TYPE) SLAG_OPERATION_TYPE,
        SLAG_OPERATION_TYPES(X)
#undef X
    };

    static constexpr size_t OPERATION_TYPE_COUNT = 0
#define X(SLAG_OPERATION_TYPE) + 1
        SLAG_OPERATION_TYPES(X)
#undef X
    ;

    size_t to_index(OperationType operation_type);
    const char* to_string(OperationType operation_type);

}
