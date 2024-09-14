#pragma once

#include <liburing.h>
#include "slag/core.h"
#include "slag/context.h"
#include "slag/system/interrupt.h"
#include "slag/system/operation.h"
#include "slag/system/file_descriptor.h"

namespace slag {

    class InterruptOperation final : public Operation {
    public:
        InterruptOperation(Ref<FileDescriptor> target_ring, const Interrupt interrupt)
            : Operation(OperationType::INTERRUPT)
            , target_ring_(target_ring)
            , interrupt_(interrupt)
            , result_(-EAGAIN)
        {
        }

        int32_t result() const {
            return result_;
        }

    private:
        void prepare_operation(struct io_uring_sqe& io_sqe) override {
            (void)io_sqe;
            abort();
            // io_uring_prep_nop(&io_sqe);
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
        Ref<FileDescriptor> target_ring_;
        Interrupt           interrupt_;
        int32_t             result_;
    };

}
