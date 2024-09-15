#pragma once

#include <cstdint>
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

    struct InterruptState {
        ThreadMask sources;
        Event      event;
    };

    using InterruptVector = std::array<InterruptState, INTERRUPT_REASON_COUNT>;

}
