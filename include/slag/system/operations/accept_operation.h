#pragma once

#include <cerrno>
#include <cassert>
#include "slag/system/multishot_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::ACCEPT> : public MultishotOperation<FileHandle> {
    public:
        Operation(Reactor& reactor, FileHandle socket)
            : MultishotOperation{OperationType::ACCEPT, reactor}
            , socket_{std::move(socket)}
        {
            assert(socket_);
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            struct sockaddr* addr = nullptr;
            socklen_t* addrlen = nullptr;
            int flags = 0;

            io_uring_prep_multishot_accept(
                &sqe,
                socket_.file_descriptor(),
                addr,
                addrlen,
                flags
            );
        }

        Result<FileHandle> handle_operation_result(int32_t result, bool more) override final {
            (void)more;

            if (result < 0) {
                return make_system_error(-result);
            }
            
            return Result<FileHandle>(FileHandle(result));
        }

    private:
        FileHandle socket_;
    };

}
