#include "slag/operation_state_machine.h"
#include <cstdlib>
#include <cassert>

static_assert(slag::OPERATION_STATE_COUNT == 7, "Update operation transitions and actions");
static_assert(slag::OPERATION_EVENT_COUNT == 4, "Update operation transitions");

const slag::OperationState slag::operation_transitions[slag::OPERATION_STATE_COUNT][slag::OPERATION_EVENT_COUNT] = {
    // SPOOLED
    {
        slag::OperationState::WORKING,        // SUBMISSION
        slag::OperationState::INVALID,        // COMPLETION
        slag::OperationState::INVALID,        // NOTIFICATION
        slag::OperationState::COMPLETE,       // CANCEL
    },
    // WORKING
    {
        slag::OperationState::INVALID,        // SUBMISSION
        slag::OperationState::COMPLETE,       // COMPLETION
        slag::OperationState::INVALID,        // NOTIFICATION
        slag::OperationState::CANCEL_SPOOLED, // CANCEL
    },
    // CANCEL_SPOOLED
    {
        slag::OperationState::CANCEL_WORKING, // SUBMISSION
        slag::OperationState::COMPLETE,       // COMPLETION
        slag::OperationState::INVALID,        // NOTIFICATION
        slag::OperationState::CANCEL_SPOOLED, // CANCEL
    },
    // CANCEL_WORKING
    {
        slag::OperationState::INVALID,        // SUBMISSION
        slag::OperationState::COMPLETE,       // COMPLETION
        slag::OperationState::INVALID,        // NOTIFICATION
        slag::OperationState::CANCEL_WORKING, // CANCEL
    },
    // COMPLETE
    {
        slag::OperationState::INVALID,        // SUBMISSION
        slag::OperationState::INVALID,        // COMPLETION
        slag::OperationState::TERMINAL,       // NOTIFICATION
        slag::OperationState::COMPLETE,       // CANCEL
    },
    // TERMINAL
    {
        slag::OperationState::INVALID,        // SUBMISSION
        slag::OperationState::INVALID,        // COMPLETION
        slag::OperationState::INVALID,        // NOTIFICATION
        slag::OperationState::INVALID,        // CANCEL
    },
    // INVALID
    {
        slag::OperationState::INVALID,        // SUBMISSION
        slag::OperationState::INVALID,        // COMPLETION
        slag::OperationState::INVALID,        // NOTIFICATION
        slag::OperationState::INVALID,        // CANCEL
    },
};

const slag::OperationAction slag::operation_actions[OPERATION_STATE_COUNT]  = {
    slag::OperationAction::SUBMIT, // SPOOLED
    slag::OperationAction::WAIT,   // WORKING
    slag::OperationAction::SUBMIT, // CANCEL_SPOOLED
    slag::OperationAction::WAIT,   // CANCEL_WORKING
    slag::OperationAction::NOTIFY, // COMPLETE
    slag::OperationAction::REMOVE, // TERMINAL
    slag::OperationAction::PANIC,  // INVALID
};

size_t slag::to_index(OperationState state) {
    return static_cast<size_t>(state);
}

size_t slag::to_index(OperationEvent event) {
    return static_cast<size_t>(event);
}

size_t slag::to_index(OperationAction action) {
    return static_cast<size_t>(action);
}

const char* slag::to_string(OperationState state) {
    switch (state) {
#define X(SLAG_OPERATION_STATE) case OperationState::SLAG_OPERATION_STATE: return #SLAG_OPERATION_STATE;
        SLAG_OPERATION_STATES(X)
#undef X
    }

    abort();
}

const char* slag::to_string(OperationEvent event) {
    switch (event) {
#define X(SLAG_OPERATION_EVENT) case OperationEvent::SLAG_OPERATION_EVENT: return #SLAG_OPERATION_EVENT;
        SLAG_OPERATION_EVENTS(X)
#undef X
    }

    abort();
}

const char* slag::to_string(OperationAction action) {
    switch (action) {
#define X(SLAG_OPERATION_ACTION) case OperationAction::SLAG_OPERATION_ACTION: return #SLAG_OPERATION_ACTION;
        SLAG_OPERATION_ACTIONS(X)
#undef X
    }

    abort();
}

slag::OperationStateMachine::OperationStateMachine()
    : state_{OperationState::SPOOLED}
{
}

slag::OperationState slag::OperationStateMachine::state() const {
    return state_;
}

slag::OperationAction slag::OperationStateMachine::action() const {
    return operation_actions[to_index(state_)];
}

void slag::OperationStateMachine::handle_event(OperationEvent event) {
    state_ = operation_transitions[to_index(state_)][to_index(event)];
}

void slag::OperationStateMachine::reset() {
    state_ = OperationState::SPOOLED;
}
