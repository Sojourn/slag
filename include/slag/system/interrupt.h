#pragma once

#include <bitset>
#include <cstdint>

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

    class InterruptHandler {
    protected:
        ~InterruptHandler() = default;

    public:
        bool is_subscribed(InterruptReason reason) const {
            return subscriptions_.test(to_index(reason));
        }

        void subscribe(InterruptReason reason) {
            subscriptions_.set(to_index(reason));
        }

        void unsubscribe(InterruptReason reason) {
            subscriptions_.reset(to_index(reason));
        }

        virtual void handle_interrupt(uint16_t source, InterruptReason reason) = 0;

    private:
        std::bitset<INTERRUPT_REASON_COUNT> subscriptions_;
    };

}
