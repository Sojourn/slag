#pragma once

#include <bitset>
#include <string>
#include <cstddef>
#include <cstdint>

#define SLAG_OPERATION_FLAGS(X) \
    X(BARRIER)                  \
    X(INTERNAL)

namespace slag {

    enum class OperationFlag : uint32_t {
#define X(SLAG_OPERATION_FLAG) SLAG_OPERATION_FLAG,
        SLAG_OPERATION_FLAGS(X)
#undef X
    };

    static constexpr size_t OPERATION_FLAG_COUNT = 0
#define X(SLAG_OPERATION_FLAG) + 1
        SLAG_OPERATION_FLAGS(X)
#undef X
    ;

    using OperationFlags = std::bitset<OPERATION_FLAG_COUNT>;

    size_t to_index(OperationFlag operation_flag);
    const char* to_string(OperationFlag operation_flag);
    std::string to_string(const OperationFlags& operation_flags);

}
