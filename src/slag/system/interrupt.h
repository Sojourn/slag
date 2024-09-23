#pragma once

#include <cstdint>
#include <cstring>
#include "slag/core.h"
#include "slag/topology.h"

#define SLAG_LIB_INTERRUPT_REASONS(X) \
    X(HALT)                           \
    X(STOP)                           \
    X(LINK)                           \

#ifndef SLAG_APP_INTERRUPT_REASONS
#  define SLAG_APP_INTERRUPT_REASONS(X)
#endif

#define SLAG_INTERRUPT_REASONS(X) \
    SLAG_LIB_INTERRUPT_REASONS(X) \
    SLAG_APP_INTERRUPT_REASONS(X) \

namespace slag {

    // An invalid operation key is used to distinguish an interrupt from a normal operation.
    constexpr OperationKey INTERRUPT_OPERATION_KEY;

    enum class InterruptReason : uint16_t {
#define X(SLAG_INTERRUPT_REASON) \
        SLAG_INTERRUPT_REASON,   \

        SLAG_INTERRUPT_REASONS(X)
#undef X
    };

    constexpr size_t INTERRUPT_REASON_COUNT = 0
#define X(SLAG_INTERRUPT_REASON) + 1
        SLAG_INTERRUPT_REASONS(X)
#undef X
    ;

    constexpr inline size_t to_index(InterruptReason reason) {
        return static_cast<size_t>(reason);
    }

    struct Interrupt {
        ThreadIndex     source;
        InterruptReason reason;
    };

    // This needs to fit within the `io_uring_sqe.len: unsigned int` field.
    static_assert(sizeof(Interrupt) <= sizeof(uint32_t));

    struct InterruptState {
        ThreadMask sources;
        Event      event;
    };

    using InterruptVector = std::array<InterruptState, INTERRUPT_REASON_COUNT>;

}
