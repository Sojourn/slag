#pragma once

#include <string_view>
#include <bitset>
#include <cstdlib>
#include <cstdint>
#include <cstddef>

#define SLAG_OPERATION_TYPES(X) \
    X(NOP)                      \
    X(CLOSE)                    \
    X(POLL_MULTISHOT)           \
    X(INTERRUPT)                \

    // X(OPEN)
    // X(CLOSE)
    // X(WRITE)
    // X(TIMER)
    // X(SOCKET)
    // X(CONNECT)
    // X(ACCEPT)
    // X(MADVISE)
    // X(INTERRUPT)

namespace slag {

    enum class OperationType : uint8_t {
#define X(SLAG_OPERATION_TYPE) \
        SLAG_OPERATION_TYPE,   \

        SLAG_OPERATION_TYPES(X)
#undef X
    };

    constexpr size_t OPERATION_TYPE_COUNT = 0
#define X(SLAG_OPERATION_TYPE) + 1
        SLAG_OPERATION_TYPES(X)
#undef X
    ;

    constexpr size_t to_index(OperationType operation_type) {
        return static_cast<size_t>(operation_type);
    }

    constexpr std::string_view to_string_view(OperationType operation_type) {
        using namespace std::literals;

        switch (operation_type) {
#define X(SLAG_OPERATION_TYPE)                         \
            case OperationType::SLAG_OPERATION_TYPE: { \
                return #SLAG_OPERATION_TYPE##sv;       \
            }                                          \

            SLAG_OPERATION_TYPES(X)
#undef X
        }

        abort();
    }

}
