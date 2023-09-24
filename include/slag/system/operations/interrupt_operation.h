#pragma once

#include <cerrno>
#include <cassert>
#include "slag/system/primitive_operation.h"

namespace slag {

    struct InterruptOperationPayload {
        uint16_t source;
        uint16_t reason;
    };

    template<>
    class Operation<OperationType::INTERRUPT> : public PrimitiveOperation<void> {
    public:
        explicit Operation(Reactor& reactor, int file_descriptor, InterruptOperationPayload payload)
            : PrimitiveOperation{OperationType::INTERRUPT, reactor}
            , file_descriptor_{file_descriptor}
            , payload_{payload}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            uint32_t payload = 0;
            uint64_t user_data = 0;
            unsigned int flags = 0;

            memcpy(&payload, &payload_, sizeof(payload_));

            io_uring_prep_msg_ring(
                &sqe,
                file_descriptor_,
                payload,
                user_data,
                flags
            );
        }

        Result<void> handle_operation_result(int32_t result, bool more) override final {
            assert(!more);

            if (result < 0) {
                return make_system_error(-result);
            }

            return {};
        }

    private:
        int                       file_descriptor_;
        InterruptOperationPayload payload_;
    };

}
