#pragma once

#include <cerrno>
#include <cassert>
#include "slag/core/service_interface.h"
#include "slag/system/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::NOP> : public PrimitiveOperation<void> {
    public:
        explicit Operation(SystemServiceInterface& system_service)
            : PrimitiveOperation{OperationType::NOP, system_service}
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
