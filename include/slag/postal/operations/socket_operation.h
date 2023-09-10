#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include "slag/postal/primitive_operation.h"

namespace slag::postal {

    template<>
    class Operation<OperationType::SOCKET> : public PrimitiveOperation<FileHandle> {
    public:
        explicit Operation(Reactor& reactor, int domain, int type, int protocol = 0)
            : PrimitiveOperation{OperationType::SOCKET, reactor}
            , domain_{domain}
            , type_{type}
            , protocol_{protocol}
        {
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            int flags = 0;

            io_uring_prep_socket(
                &sqe,
                domain_,
                type_,
                protocol_,
                flags
            );
        }

        Result<FileHandle> handle_operation_result(int32_t result) override final {
            if (result < 0) {
                return make_system_error(-result);
            }
            
            return Result<FileHandle>(FileHandle(result));
        }

    private:
        int domain_;
        int type_;
        int protocol_;
    };

}
