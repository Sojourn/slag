#pragma once

#include <cerrno>
#include <cassert>
#include "slag/postal/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::NOP> : public PrimitiveOperation<void> {
    public:
        explicit Operation(Reactor& reactor)
            : PrimitiveOperation{OperationType::NOP, reactor}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            io_uring_prep_nop(&sqe);
        }

        Result<void> handle_operation_result(int32_t result, bool more) override final {
            assert(!more);

            if (result < 0) {
                return make_system_error(-result);
            }

            return {};
        }
    };

}
