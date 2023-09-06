#pragma once

#include "operations/nop_operation.h"
#include "operations/open_operation.h"
#include "operations/close_operation.h"
#include "operations/write_operation.h"

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
