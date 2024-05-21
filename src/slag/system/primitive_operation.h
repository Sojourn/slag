#pragma once

#include <liburing.h>

#include "slag/system/operation.h"

namespace slag {

    class PrimitiveOperation
        : public Operation
        , public Pollable<PollableType::COMPLETE>
    {
    protected:
        explicit PrimitiveOperation(const OperationType operation_type)
            : Operation(operation_type)
            , state_(State::OPERATION_PENDING)
        {
            init(operation_user_data_);
            init(cancel_user_data_);
        }

        void prepare(struct io_uring_sqe& sqe, OperationUserData& user_data) final {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    state_ = State::OPERATION_WORKING;
                    memcpy(&operation_user_data_, &user_data, sizeof(operation_user_data_));
                    prepare_operation(sqe, user_data);
                    break;
                }
                case State::CANCEL_PENDING: {
                    state_ = State::CANCEL_WORKING;
                    memcpy(&cancel_user_data_, &user_data, sizeof(cancel_user_data_));
                    prepare_cancel(sqe, user_data);
                    break;
                }
                default: {
                    abort();
                }
            }

            writable_event_.reset();
        }

        virtual void prepare_operation(struct io_uring_sqe& sqe, OperationUserData& user_data) = 0;

        void prepare_cancel(struct io_uring_sqe& sqe, OperationUserData& user_data) {
            io_uring_prep_cancel64(&sqe, encode(operation_user_data_), 0);
        }

        void handle_result(int32_t result, bool complete, OperationUserData user_data) final {
            assert(user_data.type == operation_type());
            assert(complete || user_data.flags.multishot);

            const bool is_operation_result = memcmp(&operation_user_data_, &user_data, sizeof(user_data)) == 0;
            const bool is_cancel_result = memcmp(&cancel_user_data_, &user_data, sizeof(user_data)) == 0;

            if (is_operation_result) {
                handle_operation_result(result, complete, user_data);
            }
            else if (is_cancel_result) {
                handle_cancel_result(result, complete, user_data);
            }
            else {
                abort();
            }

            if (is_quiescent()) {
                state_ = State::COMPLETE;
                complete_event_.set();
            }
        }

        virtual void handle_operation_result(int32_t result, bool complete, OperationUserData user_data) = 0;

        virtual void handle_cancel_result(int32_t result, bool complete, OperationUserData user_data) {
            (void)result;
            (void)complete;
            (void)user_data;
        }

    private:
        enum class State {
            OPERATION_PENDING, // Waiting to submit the operation.
            OPERATION_WORKING, // Waiting for the operation to complete.
            CANCEL_PENDING,    // Waiting to submit the cancel operation.
            CANCEL_WORKING,    // Waiting for the cancel operation to complete.
            COMPLETE,          // The operation has completed.
        };

        State             state_;
        Event             writable_event_;
        Event             complete_event_;
        OperationUserData operation_user_data_;
        OperationUserData cancel_user_data_;
    };

}
