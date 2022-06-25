#include "catch.hpp"
#include "slag/slag.h"
#include <string>

using namespace slag;

TEST_CASE("normal operation state machine transitions", "[OperationStateMachine]") {
    OperationStateMachine stm;

    CHECK(stm.state() == OperationState::SPOOLED);
    CHECK(stm.action() == OperationAction::SUBMIT);

    SECTION("normal") {
        stm.handle_event(OperationEvent::SUBMISSION);
        CHECK(stm.state() == OperationState::WORKING);
        CHECK(stm.action() == OperationAction::WAIT);

        stm.handle_event(OperationEvent::COMPLETION);
        CHECK(stm.state() == OperationState::COMPLETE);
        CHECK(stm.action() == OperationAction::NOTIFY);

        stm.handle_event(OperationEvent::NOTIFICATION);
        CHECK(stm.state() == OperationState::TERMINAL);
        CHECK(stm.action() == OperationAction::REMOVE);
    }

    SECTION("cancel spooled") {
        stm.handle_event(OperationEvent::CANCEL);
        CHECK(stm.state() == OperationState::COMPLETE);
        CHECK(stm.action() == OperationAction::NOTIFY);

        stm.handle_event(OperationEvent::NOTIFICATION);
        CHECK(stm.state() == OperationState::TERMINAL);
        CHECK(stm.action() == OperationAction::REMOVE);
    }

    SECTION("cancel working") {
        stm.handle_event(OperationEvent::SUBMISSION);
        CHECK(stm.state() == OperationState::WORKING);
        CHECK(stm.action() == OperationAction::WAIT);

        stm.handle_event(OperationEvent::CANCEL);
        CHECK(stm.state() == OperationState::CANCEL_SPOOLED);
        CHECK(stm.action() == OperationAction::SUBMIT);

        stm.handle_event(OperationEvent::SUBMISSION);
        CHECK(stm.state() == OperationState::CANCEL_WORKING);
        CHECK(stm.action() == OperationAction::WAIT);

        // either the result or the cancelled response
        stm.handle_event(OperationEvent::COMPLETION);
        CHECK(stm.state() == OperationState::COMPLETE);
        CHECK(stm.action() == OperationAction::NOTIFY);

        stm.handle_event(OperationEvent::NOTIFICATION);
        CHECK(stm.state() == OperationState::TERMINAL);
        CHECK(stm.action() == OperationAction::REMOVE);
    }

    SECTION("cancel working race") {
        stm.handle_event(OperationEvent::SUBMISSION);
        CHECK(stm.state() == OperationState::WORKING);
        CHECK(stm.action() == OperationAction::WAIT);

        stm.handle_event(OperationEvent::CANCEL);
        CHECK(stm.state() == OperationState::CANCEL_SPOOLED);
        CHECK(stm.action() == OperationAction::SUBMIT);

        // completion event before the cancel has been submitted
        stm.handle_event(OperationEvent::COMPLETION);
        CHECK(stm.state() == OperationState::COMPLETE);
        CHECK(stm.action() == OperationAction::NOTIFY);

        stm.handle_event(OperationEvent::NOTIFICATION);
        CHECK(stm.state() == OperationState::TERMINAL);
        CHECK(stm.action() == OperationAction::REMOVE);
    }
}
