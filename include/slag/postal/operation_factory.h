#pragma once

#include "slag/address.h"
#include "slag/postal/domain.h"
#include "slag/postal/reactor.h"
#include "slag/postal/operation.h"
#include "slag/postal/operation_handle.h"

namespace slag::postal {

    using NopOperationHandle        = OperationHandle<OperationType::NOP>;
    using OpenOperationHandle       = OperationHandle<OperationType::OPEN>;
    using CloseOperationHandle      = OperationHandle<OperationType::CLOSE>;
    using WriteOperationHandle      = OperationHandle<OperationType::WRITE>;
    using TimerOperationHandle      = OperationHandle<OperationType::TIMER>;
    using SocketOperationHandle     = OperationHandle<OperationType::SOCKET>;
    using ConnectOperationHandle    = OperationHandle<OperationType::CONNECT>;
    using AcceptOperationHandle     = OperationHandle<OperationType::ACCEPT>;
    using MAdviseOperationHandle    = OperationHandle<OperationType::MADVISE>;
    using InterruptOperationHandle  = OperationHandle<OperationType::INTERRUPT>;

    template<OperationType type, typename... Args>
    inline OperationHandle<type> make_operation(Args&&... args) {
        Reactor& reactor = region().reactor();
        return reactor.start_operation<type>(
            reactor,
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    inline auto make_nop_operation(Args&&... args) {
        return make_operation<OperationType::NOP>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto make_open_operation(Args&&... args) {
        return make_operation<OperationType::OPEN>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto make_close_operation(Args&&... args) {
        return make_operation<OperationType::CLOSE>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto make_write_operation(Args&&... args) {
        return make_operation<OperationType::WRITE>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto make_timer_operation(Args&&... args) {
        return make_operation<OperationType::TIMER>(std::forward<Args>(args)...);
    }

    inline auto make_socket_operation(int domain, int type, int protocol = 0) {
        return make_operation<OperationType::SOCKET>(domain, type, protocol);
    }

    inline auto make_connect_operation(FileHandle socket, const Address& address) {
        return make_operation<OperationType::CONNECT>(std::move(socket), address);
    }

    inline auto make_accept_operation(FileHandle socket) {
        return make_operation<OperationType::ACCEPT>(std::move(socket));
    }

    inline auto make_madvise_operation(void* address, size_t length, int advice) {
        return make_operation<OperationType::MADVISE>(address, length, advice);
    }

    inline auto make_interrupt_operation(int file_descriptor, InterruptOperationPayload payload) {
        return make_operation<OperationType::INTERRUPT>(file_descriptor, payload);
    }

}
