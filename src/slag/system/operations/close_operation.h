#pragma once

#include <liburing.h>
#include "slag/core.h"
#include "slag/system/operation.h"

namespace slag {

    class CloseOperation final : public Operation {
    public:
        CloseOperation(int file_descriptor)
            : Operation(OperationType::CLOSE)
            , file_descriptor_(file_descriptor)
            , result_(-EAGAIN)
        {
        }

        int32_t result() const {
            return result_;
        }

    private:
        void prepare_operation(struct io_uring_sqe& io_sqe) override {
            io_uring_prep_close(&io_sqe, file_descriptor_);
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
        int     file_descriptor_;
        int32_t result_;
    };

}
