#include <cassert>

namespace slag {

    template<OperationType operation_type, typename... Args>
    inline OperationHandle<operation_type> Reactor::start_operation(Args&&... args) {
        Operation<operation_type>& operation = operation_allocator_.allocate<operation_type>(std::forward<Args>(args)...);

        // Wait for the operation to have something to submit.
        {
            OperationBase& operation_base = operation;
            pending_submissions_.insert<PollableType::WRITABLE>(operation_base);
        }

        // Schedule the operation if it is derived from Task.
        if constexpr (std::is_base_of_v<Task, Operation<operation_type>>) {
            executor_.insert(operation);
        }

        // Update metrics for this operation.
        increment_operation_count(operation);

        return OperationHandle<operation_type>(operation);
    }

}
