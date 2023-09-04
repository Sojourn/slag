#pragma once

#include <cerrno>
#include "slag/logging.h"
#include "slag/postal/operation_base.h"

namespace slag::postal {

    template<>
    class Operation<OperationType::NOP> : public PrimitiveOperation {
    public:
        explicit Operation(Reactor& reactor)
            : PrimitiveOperation{OperationType::NOP, reactor}
            , state_{State::OPERATION_PENDING}
            , operation_slot_{-1}
            , cancel_slot_{-1}
            , result_{-EAGAIN}
        {
            writable_event().set();
        }

        int32_t result() const {
            return result_;
        }

        void cancel() override final {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    operation_slot_ = SLOT_COUNT;
                    handle_result(operation_slot_, -ECANCELED);
                    break;
                }
                case State::OPERATION_WORKING: {
                    // Signal to the reactor that we want to submit something else (the cancel).
                    state_ = State::CANCEL_PENDING;
                    writable_event().set();
                    break;
                }
                default: {
                    // Complete, or in the process of being canceled already.
                    break;
                }
            }
        }

        Event& complete_event() override final {
            return complete_event_;
        }

    private:
        void prepare(Slot slot, struct io_uring_sqe& sqe) override final {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    state_ = State::OPERATION_WORKING;
                    operation_slot_ = slot;            
                    io_uring_prep_nop(&sqe);
                    break;
                }
                case State::CANCEL_PENDING: {
                    state_ = State::CANCEL_WORKING;
                    cancel_slot_ = slot;
                    io_uring_prep_cancel(&sqe, make_user_data(operation_slot_), 0);
                    break;
                }
                default: {
                    abort();
                }
            }

            writable_event().reset();
        }

        void handle_result(Slot slot, int32_t result) override final {
            if (slot == operation_slot_) {
                operation_slot_ = -1;
                handle_operation_result(result);
            }
            else if (slot == cancel_slot_) {
                cancel_slot_ = -1;
                handle_cancel_result(result);
            }
            else {
                assert(false);
            }

            // Check if all of our in-flight requests have completed.
            if (is_quiescent()) {
                state_ = State::COMPLETE;
                complete_event_.set();
            }
        }

        void handle_operation_result(int32_t result) {
            result_ = result;
            complete_event_.set();
        }

        void handle_cancel_result(int32_t result) {
            (void)result;
        }

    private:
        enum class State {
            OPERATION_PENDING,
            OPERATION_WORKING,
            CANCEL_PENDING,
            CANCEL_WORKING,
            COMPLETE,
        };

        State   state_;
        Slot    operation_slot_;
        Slot    cancel_slot_;
        int32_t result_;
        Event   complete_event_;
    };

}
