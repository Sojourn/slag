#pragma once

#include <cstdint>
#include "mantle/mantle.h"

#define SLAG_INTERRUPT_REASONS(X) \
    X(HALT)                       \
    X(STOP)                       \

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
        mantle::RegionId sender;
        InterruptReason  reason;
    };

    class InterruptHandler {
    public:
        virtual ~InterruptHandler() = default;

        virtual void handle_interrupt(Interrupt interrupt) = 0;
    };

}
