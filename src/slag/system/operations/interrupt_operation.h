#pragma once

#include <liburing.h>
#include "slag/core.h"
#include "slag/context.h"
#include "slag/system/reactor.h"
#include "slag/system/interrupt.h"
#include "slag/system/operation.h"
#include "slag/system/file_descriptor.h"

namespace slag {

    class InterruptOperation final : public Operation {
    public:
        InterruptOperation(std::shared_ptr<Reactor> reactor, const Interrupt interrupt)
            : Operation(OperationType::INTERRUPT)
            , reactor_(std::move(reactor))
            , interrupt_(interrupt)
            , result_(-EAGAIN)
        {
        }

        int32_t result() const {
            return result_;
        }

    private:
        void prepare_operation(struct io_uring_sqe& io_sqe) override {
            static constexpr int flags = 0;

            unsigned int encoded_interrupt;
            memcpy(&encoded_interrupt, &interrupt_, sizeof(encoded_interrupt));

            io_uring_prep_msg_ring(
                &io_sqe,
                reactor_->borrow_file_descriptor(),
                encoded_interrupt,
                encode_operation_key(INTERRUPT_OPERATION_KEY),
                flags
            );
        }

        void handle_operation_result(int32_t result, bool more) override {
            assert(!more);

            result_ = result;
        }

        void handle_cancel_result(int32_t result, bool more) override {
            assert(!more);

            if (result >= 0) {
                result_ = -ECANCELED;
            }
        }

    private:
        std::shared_ptr<Reactor> reactor_;
        Interrupt                interrupt_;
        int32_t                  result_;
    };

}
