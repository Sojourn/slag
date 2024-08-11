#pragma once

#include <bitset>
#include "slag/core.h"
#include "operation_types.h"
#include "operation_user_data.h"

namespace slag {

    template<>
    class Resource<ResourceType::OPERATION>
        : public ResourceBase
        , public Pollable<PollableType::COMPLETE> // Ready after the operation has completed.
        , public Pollable<PollableType::READABLE> // Ready when the result is partially or fully available.
        , public Pollable<PollableType::WRITABLE> // Ready when the operation can be submitted. Used internally.
    {
    public:
        explicit Resource(const OperationType operation_type)
            : ResourceBase(ResourceType::OPERATION)
            , operation_type_(operation_type)
            , abandoned_(false)
            , daemonized_(false)
            , next_slot_(0)
        {
        }

        [[nodiscard]]
        OperationType operation_type() const {
            return operation_type_;
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

        [[nodiscard]]
        bool is_quiescent() const {
            return working_count_ == 0;
        }

        OperationSlot acquire_slot() {
            assert(next_slot_ < std::numeric_limits<OperationSlot>::max());

            working_count_ += 1;
            return next_slot_++;
        }

        void release_slot(OperationSlot slot) {
            assert(working_count_ > 0);

            working_count_ -= 1;
        }

        void cancel() {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    // The operation has not been submitted yet, and can be canceled immediately.
                    constexpr bool complete = true;
                    constexpr bool synthetic = true;
                    handle_result(-ECANCELED, complete, synthetic, operation_user_data_);
                    break;
                }
                case State::OPERATION_WORKING: {
                    state_ = State::CANCEL_PENDING;
                    writable_event_.set();
                    break;
                }
                default: {
                    break;
                }
            }
        }

        void prepare(struct io_uring_sqe& sqe, OperationUserData& user_data) {
            switch (state_) {
                case State::OPERATION_PENDING: {
                    prepare_operation(sqe, user_data);
                    break;
                }
                case State::CANCEL_PENDING: {
                    prepare_cancel(sqe, user_data);
                    break;
                }
                default: {
                    abort();
                }
            }

            working_count_ += 1;
        }

        void handle_result(int32_t result, bool complete, bool synthetic, OperationUserData user_data) {
            if (!synthetic) {
                working_count_ -= 1;
            }

            if (user_data == operation_user_data_) {
                handle_operation_result(result, complete, user_data);
            }
            else if (user_data == cancel_user_data_) {
                handle_cancel_result(result, complete, user_data);
            }
            else {
                abort();
            }
        }

    private:
        virtual void prepare_operation(struct io_uring_sqe& sqe, OperationUserData& user_data);

        void prepare_cancel(struct io_uring_sqe& sqe, OperationUserData& user_data) {
            io_uring_prep_cancel64(&sqe, encode(operation_user_data_), 0);
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

        OperationType   operation_type_;
        State           state_;

        bool            abandoned_;
        bool            daemonized_;

        OperationSlot     next_slot_;
        uint8_t           working_count_;

        OperationUserData operation_user_data_;
        OperationUserData cancel_user_data_;

        Event           writable_event_;
        Event           complete_event_;
    };

    using Operation       = Resource<ResourceType::OPERATION>;
    using OperationHandle = mantle::Handle<Operation>;

}
