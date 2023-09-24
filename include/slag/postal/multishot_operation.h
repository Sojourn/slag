#pragma once

#include <cassert>
#include "slag/error.h"
#include "slag/result.h"
#include "slag/core/event.h"
#include "slag/core/pollable.h"
#include "slag/core/pollable/pollable_queue.h"
#include "slag/postal/operation_base.h"

namespace slag {

    template<typename T>
    class MultishotOperation
        : public OperationBase
        , public Pollable<PollableType::COMPLETE>
    {
    public:
        MultishotOperation(OperationType type, Reactor& reactor)
            : OperationBase{type, reactor}
            , state_{State::OPERATION_PENDING}
            , operation_slot_{-1}
            , cancel_slot_{-1}
        {
            writable_event().set();
        }

        PollableQueue<Result<T>>& results() {
            return results_;
        }

        Event& readable_event() override final {
            return results_.readable_event();
        }

        Event& complete_event() override final {
            return complete_event_;
        }

        void cancel() override {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    operation_slot_ = SLOT_COUNT;
                    handle_result(operation_slot_, -ECANCELED, false);
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

    private:
        void prepare(Slot slot, struct io_uring_sqe& sqe) override final {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    state_ = State::OPERATION_WORKING;
                    operation_slot_ = slot;            
                    prepare_operation(sqe);
                    break;
                }
                case State::CANCEL_PENDING: {
                    state_ = State::CANCEL_WORKING;
                    cancel_slot_ = slot;
                    prepare_cancel(sqe);
                    break;
                }
                default: {
                    abort();
                }
            }

            writable_event().reset();
        }

        virtual void prepare_operation(struct io_uring_sqe& sqe) = 0;

        void prepare_cancel(struct io_uring_sqe& sqe) {
            io_uring_prep_cancel(&sqe, make_user_data(operation_slot_), 0);
        }

    private:
        void handle_result(Slot slot, int32_t result, bool more) override final {
            if (slot == operation_slot_) {
                if (!more) {
                    operation_slot_ = -1;
                }

                results_.push_back(handle_operation_result(result, more));
            }
            else if (slot == cancel_slot_) {
                if (!more) {
                    cancel_slot_ = -1;
                }

                handle_cancel_result(result, more);
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

        virtual Result<T> handle_operation_result(int32_t result, bool more) = 0;

        void handle_cancel_result(int32_t result, bool more) {
            (void)result;
            (void)more;
        }

    private:
        enum class State {
            OPERATION_PENDING,
            OPERATION_WORKING,
            CANCEL_PENDING,
            CANCEL_WORKING,
            COMPLETE,
        };

        State                    state_;
        PollableQueue<Result<T>> results_;
        Slot                     operation_slot_;
        Slot                     cancel_slot_;
        Event                    complete_event_;
    };

}
