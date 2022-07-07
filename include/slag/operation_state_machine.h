#pragma once

#include <cstdint>
#include <cstddef>

#define SLAG_OPERATION_STATES(X) \
    X(SPOOLED)                   \
    X(WORKING)                   \
    X(CANCEL_SPOOLED)            \
    X(CANCEL_WORKING)            \
    X(COMPLETE)                  \
    X(TERMINAL)                  \
    X(INVALID)

#define SLAG_OPERATION_EVENTS(X) \
    X(SUBMISSION)                \
    X(COMPLETION)                \
    X(NOTIFICATION)              \
    X(CANCEL)

#define SLAG_OPERATION_ACTIONS(X) \
    X(WAIT)                       \
    X(SUBMIT)                     \
    X(NOTIFY)                     \
    X(REMOVE)                     \
    X(PANIC)

namespace slag {

    enum class OperationState : uint8_t {
#define X(SLAG_OPERATION_STATE) SLAG_OPERATION_STATE,
        SLAG_OPERATION_STATES(X)
#undef X
    };

    enum class OperationEvent : uint8_t {
#define X(SLAG_OPERATION_EVENT) SLAG_OPERATION_EVENT,
        SLAG_OPERATION_EVENTS(X)
#undef X
    };

    enum class OperationAction : uint8_t {
#define X(SLAG_OPERATION_ACTION) SLAG_OPERATION_ACTION,
        SLAG_OPERATION_ACTIONS(X)
#undef X
    };

    static constexpr size_t OPERATION_STATE_COUNT = 0
#define X(SLAG_OPERATION_STATE) + 1
        SLAG_OPERATION_STATES(X)
#undef X
    ;

    static constexpr size_t OPERATION_EVENT_COUNT = 0
#define X(SLAG_OPERATION_EVENT) + 1
        SLAG_OPERATION_EVENTS(X)
#undef X
    ;

    static constexpr size_t OPERATION_ACTION_COUNT = 0
#define X(SLAG_OPERATION_ACTION) + 1
        SLAG_OPERATION_ACTIONS(X)
#undef X
    ;

    extern const OperationState operation_transitions[OPERATION_STATE_COUNT][OPERATION_EVENT_COUNT];
    extern const OperationAction operation_actions[OPERATION_STATE_COUNT];

    size_t to_index(OperationState state);
    size_t to_index(OperationEvent event);
    size_t to_index(OperationAction action);

    const char* to_string(OperationState state);
    const char* to_string(OperationEvent event);
    const char* to_string(OperationAction action);

    // This could take a pointer to a transition table (if reactors can't all use the same one).
    //
    class OperationStateMachine {
    public:
        OperationStateMachine();

        [[nodiscard]] OperationState state() const;
        [[nodiscard]] OperationAction action() const;

        void handle_event(OperationEvent event);
        void reset();

    private:
        OperationState state_;
    };

}
