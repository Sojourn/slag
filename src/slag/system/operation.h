#pragma once

#include <liburing.h>
#include "slag/core.h"
#include "operation_types.h"
#include "operation_table.h"

namespace slag {

    enum class OperationState {
        OPERATION_PENDING, // Waiting to submit the operation.
        OPERATION_WORKING, // Waiting for the operation to complete.
        CANCEL_PENDING,    // Waiting to submit the cancel operation.
        CANCEL_WORKING,    // Waiting for the cancel operation to complete.
        COMPLETE,          // The operation has completed.
    };

    template<>
    class Resource<ResourceType::OPERATION> 
        : public Object
        , public Pollable<PollableType::COMPLETE> // Ready after the operation has completed.
        , public Pollable<PollableType::READABLE> // Ready when the result is partially or fully available.
        , public Pollable<PollableType::WRITABLE> // Ready when the operation can be submitted. Used internally.
    {
    public:
        explicit Resource(const OperationType operation_type)
            : Object(static_cast<ObjectGroup>(ResourceType::OPERATION))
            , type_(operation_type)
            , state_(OperationState::OPERATION_PENDING)
            , abandoned_(false)
            , daemonized_(false)
        {
            writable_event_.set();
        }

        [[nodiscard]]
        OperationType type() const {
            return type_;
        }

        [[nodiscard]]
        OperationState state() const {
            return state_;
        }

        [[nodiscard]]
        bool is_quiescent() const {
            if (normal_op_key_ || cancel_op_key_) {
                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool is_abandoned() const {
            return abandoned_;
        }

        void abandon() {
            abandoned_ = true;
        }

        [[nodiscard]]
        bool is_daemonized() const {
            return daemonized_;
        }

        void daemonize() {
            daemonized_ = true;
        }

        void cancel() {
            switch (state_) {
                case OperationState::OPERATION_PENDING: {
                    // The operation has not been submitted yet, and can be canceled immediately.
                    constexpr bool more = false;
                    handle_result(normal_op_key_, -ECANCELED, more);
                    break;
                }
                case OperationState::OPERATION_WORKING: {
                    state_ = OperationState::CANCEL_PENDING;
                    writable_event_.set();
                    break;
                }
                default: {
                    break;
                }
            }
        }

        void prepare(OperationKey op_key, struct io_uring_sqe& io_sqe) {
            switch (state_) {
                case OperationState::OPERATION_PENDING: {
                    prepare_operation(io_sqe);
                    normal_op_key_ = op_key;
                    break;
                }
                case OperationState::CANCEL_PENDING: {
                    prepare_cancel(io_sqe);
                    cancel_op_key_ = op_key;
                    break;
                }
                default: {
                    abort();
                }
            }
        }

        void handle_result(OperationKey op_key, int32_t result, bool more) {
            if (normal_op_key_ == op_key) {
                if (!more) {
                    normal_op_key_ = OperationKey{};
                }

                handle_operation_result(result, more);
            }
            else if (cancel_op_key_ == op_key) {
                if (!more) {
                    cancel_op_key_ = OperationKey{};
                }

                handle_cancel_result(result, more);
            }
            else {
                abort();
            }

            if (is_quiescent()) {
                state_ = OperationState::COMPLETE;
                complete_event_.set();
            }
        }

    private:
        virtual void prepare_operation(struct io_uring_sqe& io_sqe) = 0;

        virtual void prepare_cancel(struct io_uring_sqe& io_sqe) {
            io_uring_prep_cancel64(&io_sqe, encode_operation_key(normal_op_key_), 0);
        }

        virtual void handle_operation_result(int32_t result, bool more) = 0;

        virtual void handle_cancel_result(int32_t result, bool more) {
            (void)result;
            (void)more;
        }

    private:
        OperationType   type_;
        OperationState  state_;
        bool            abandoned_;
        bool            daemonized_;
        OperationKey    normal_op_key_;
        OperationKey    cancel_op_key_;
        Event           writable_event_;
        Event           complete_event_;
    };

}
