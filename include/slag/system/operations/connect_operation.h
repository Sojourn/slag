#pragma once

#include <cerrno>
#include <cassert>
#include "slag/address.h"
#include "slag/postal/file_handle.h"
#include "slag/core/service_interface.h"
#include "slag/system/primitive_operation.h"

namespace slag {

    template<>
    class Operation<OperationType::CONNECT> : public PrimitiveOperation<void> {
    public:
        explicit Operation(SystemServiceInterface& system_service, FileHandle socket, const Address& address)
            : PrimitiveOperation{OperationType::CONNECT, system_service}
            , socket_{std::move(socket)}
            , address_{address}
        {
            assert(socket_);
        }

    private:
        void prepare_operation(struct io_uring_sqe& sqe) override final {
            io_uring_prep_connect(
                &sqe,
                socket_.file_descriptor(),
                &address_.addr(),
                address_.size()
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
        FileHandle socket_;
        Address    address_;
    };

}
