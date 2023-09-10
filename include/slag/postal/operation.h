#pragma once

// Steps to add a new operation:
//   1. slag/postal/operation.h: Add the include.
//   2. slag/postal/operation_types.h: Add a new value to the X-macro.
//   3. slag/postal/operation_factory.h: Add a factory function to construct it.
//   4. slag/postal/operations/foo_operation.h: Create and implement.
//

#include "operations/nop_operation.h"
#include "operations/open_operation.h"
#include "operations/close_operation.h"
#include "operations/write_operation.h"
#include "operations/timer_operation.h"
#include "operations/socket_operation.h"
#include "operations/connect_operation.h"
#include "operations/accept_operation.h"

namespace slag::postal {

    template<typename Visitor>
    inline void visit(Visitor&& visitor, OperationBase& operation_base) {
        switch (operation_base.type()) {
    #define X(SLAG_POSTAL_OPERATION_TYPE) \
            case OperationType::SLAG_POSTAL_OPERATION_TYPE: {                                                \
                visitor(static_cast<Operation<OperationType::SLAG_POSTAL_OPERATION_TYPE>&>(operation_base)); \
                break;                                                                                       \
            }                                                                                                \

        SLAG_POSTAL_OPERATION_TYPES(X)
    #undef X
        }
    }

}
