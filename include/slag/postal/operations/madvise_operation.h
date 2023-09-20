#pragma once

#include <cerrno>
#include <cassert>
#include "slag/postal/primitive_operation.h"

namespace slag::postal {

    template<>
    class Operation<OperationType::MADVISE> : public PrimitiveOperation<void> {
    public:
        explicit Operation(Reactor& reactor, void* address, size_t length, int advice)
            : PrimitiveOperation{OperationType::MADVISE, reactor}
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
