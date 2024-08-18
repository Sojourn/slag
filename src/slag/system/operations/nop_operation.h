#pragma once

#include <liburing.h>
#include "slag/core.h"
#include "slag/system/operation.h"

namespace slag {

    class NopOperation final : public Operation {
    public:
        NopOperation()
            : Operation(OperationType::NOP)
            , result_(-EAGAIN)
        {
        }

        int32_t result() const {
            return result_;
        }

    private:
        void prepare_operation(struct io_uring_sqe& io_sqe) override {
            io_uring_prep_nop(&io_sqe);
        }

        void handle_operation_result(int32_t result, bool more) override {
            assert(!more);

            result_ = result;
        }

    private:
        int32_t result_;
    };

}
