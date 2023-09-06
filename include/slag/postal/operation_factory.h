#pragma once

#include "slag/postal/domain.h"
#include "slag/postal/reactor.h"
#include "slag/postal/operation.h"
#include "slag/postal/operation_handle.h"

namespace slag::postal {

    using NopOperationHandle = OperationHandle<OperationType::NOP>;
    using OpenOperationHandle = OperationHandle<OperationType::OPEN>;
    using CloseOperationHandle = OperationHandle<OperationType::CLOSE>;
    using WriteOperationHandle = OperationHandle<OperationType::WRITE>;

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

}
