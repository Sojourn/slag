#pragma once

#include <utility>
#include "slag/pool_allocator.h"
#include "slag/postal/operation_types.h"
#include "slag/postal/operation.h"

namespace slag::postal {

    class OperationAllocator {
    public:
        template<OperationType type, typename... Args>
        Operation<type>& allocate(Args&&... args) {
            auto&& allocator = std::get<to_index(type)>(allocators_);
            return allocator.allocate(std::forward<Args>(args)...);
        }

        template<OperationType type>
        void deallocate(Operation<type>& operation) {
            auto&& allocator = std::get<to_index(type)>(allocators_);
            allocator.deallocate(operation);
        }

    private:
        std::tuple<
#define X(SLAG_POSTAL_OPERATION_TYPE)                                            \
            PoolAllocator<Operation<OperationType::SLAG_POSTAL_OPERATION_TYPE>>, \

        SLAG_POSTAL_OPERATION_TYPES(X)
#undef X
            int
        > allocators_;
    };

}
