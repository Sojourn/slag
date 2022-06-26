#pragma once

#include <cstddef>

#define SLAG_OPERATION_TYPES(X) \
    X(NOP)                      \
    X(ASSIGN_FILE_DESCRIPTOR)   \
    X(CLOSE_FILE_DESCRIPTOR)

namespace slag {

    enum class OperationTypes : uint8_t {
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
