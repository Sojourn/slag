#pragma once

#include <liburing.h>

#include "slag/system/operation.h"

namespace slag {

    class PrimitiveOperation
        : public Operation
        , public Pollable<PollableType::COMPLETE>
    {
    protected:
        explicit PrimitiveOperation(OperationType operation_type)
            : Operation(operation_type)
            , state_(State::OPERATION_PENDING)
        {
            init(operation_user_data_);
            init(cancel_user_data_);
        }

        void prepare(struct io_uring_sqe& io_sqe, OperationKey op_key) final {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    state_ = State::OPERATION_WORKING;
                    operation_key_ = op_key;
                    prepare_operation(io_sqe, op_key);
                    break;
                }
                case State::CANCEL_PENDING: {
                    state_ = State::CANCEL_WORKING;
                    cancel_key_ = op_key;
                    prepare_cancel(io_sqe, op_key);
                    break;
                }
                default: {
                    abort();
                }
            }

            writable_event_.reset();
        }

        virtual void prepare_operation(struct io_uring_sqe& io_sqe, OperationKey op_key) = 0;

        void prepare_cancel(struct io_uring_sqe& io_sqe, OperationKey op_key) {
            io_uring_prep_cancel64(&io_sqe, encode_operation_key(op_key), 0);
        }

        void handle_result(int32_t result, bool more, OperationKey op_key) final {
            if (operation_key_ == op_key) {
                if (!more) {
                    operation_key_ = OperationKey{};
                }

                handle_operation_result(result, more, op_key);
            }
            else if (cancel_key_ == op_key) {
                if (!more) {
                    cancel_key_ = OperationKey{};
                }

                handle_cancel_result(result, more, op_key);
            }
            else {
                abort();
            }

            if (is_quiescent()) {
                state_ = State::COMPLETE;
                complete_event_.set();
            }
        }

        virtual void handle_operation_result(int32_t result, bool more, OperationKey op_key) = 0;
        virtual void handle_cancel_result(int32_t result, bool more, OperationKey op_key) = 0;

    private:
        enum class State {
            OPERATION_PENDING, // Waiting to submit the operation.
            OPERATION_WORKING, // Waiting for the operation to complete.
            CANCEL_PENDING,    // Waiting to submit the cancel operation.
            CANCEL_WORKING,    // Waiting for the cancel operation to complete.
            COMPLETE,          // The operation has completed.
        };

        State        state_;
        Event        writable_event_;
        Event        complete_event_;
        OperationKey operation_key_;
        OperationKey cancel_key_;
    };

}
