#pragma once

#include <cstdint>

namespace slag {

#if 1
    using InterruptReason = uint16_t;
#else
    // enum class InterruptReason : uint16_t {
    // };
#endif

    class InterruptHandler {
    protected:
        ~InterruptHandler() = default;

    public:
        virtual void handle_interrupt(uint16_t source, InterruptReason reason) = 0;
    };

}
