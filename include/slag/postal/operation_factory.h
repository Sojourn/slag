#pragma once

#include "slag/postal/domain.h"
#include "slag/postal/reactor.h"
#include "slag/postal/operation.h"
#include "slag/postal/operation_handle.h"

namespace slag::postal {

    using NopOperationHandle = OperationHandle<OperationType::NOP>;

    template<OperationType type, typename... Args>
    inline OperationHandle<type> make_operation(Args&&... args) {
        Reactor& reactor = region().reactor();
        return reactor.start_operation<type>(
            reactor,
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    inline NopOperationHandle make_nop_operation(Args&&... args) {
        return make_operation<OperationType::NOP>(std::forward<Args>(args)...);
    }

}
