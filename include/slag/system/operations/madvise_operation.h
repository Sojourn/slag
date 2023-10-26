#pragma once

#include <cerrno>
#include <cassert>
#include "slag/core/service_interface.h"
#include "slag/system/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::MADVISE> : public PrimitiveOperation<void> {
    public:
        explicit Operation(SystemServiceInterface& system_service, void* address, size_t length, int advice)
            : PrimitiveOperation{OperationType::MADVISE, system_service}
            , address_{address}
            , length_{length}
            , advice_{advice}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            io_uring_prep_madvise(&sqe, address_, length_, advice_);
        }

        Result<void> handle_operation_result(int32_t result, bool more) override final {
            assert(!more);

            if (result < 0) {
                return make_system_error(-result);
            }

            return {};
        }

    private:
        void*  address_;
        size_t length_;
        int    advice_;
    };

}
