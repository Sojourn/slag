#pragma once

#include <cerrno>
#include "slag/postal/primitive_operation.h"

namespace slag::postal {

    template<>
    class Operation<OperationType::CLOSE> : public PrimitiveOperation<void> {
    public:
        explicit Operation(Reactor& reactor, int file_descriptor)
            : PrimitiveOperation{OperationType::CLOSE, reactor}
            , file_descriptor_{file_descriptor}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            io_uring_prep_close(&sqe, file_descriptor_);
        }

        Result<void> handle_operation_result(int32_t result) override final {
            if (result < 0) {
                return make_system_error(-result);
            }

            return {};
        }

    private:
        int file_descriptor_;
    };

}
