#include <cassert>

namespace slag::postal {

    template<OperationType type, typename... Args>
    inline OperationHandle<type> Reactor::start_operation(Args&&... args) {
        Operation<type>& operation = operation_allocator_.allocate<type>(std::forward<Args>(args)...);

        // Wait for the operation to have something to submit.
        {
            OperationBase& operation_base = operation;
            pending_submissions_.insert<PollableType::WRITABLE>(operation_base);
        }

        // Schedule the operation if it is derived from Task.
        if constexpr (std::is_base_of_v<Task, Operation<type>>) {
            executor_.insert(operation);
        }

        metrics_.total_operation_count += 1;
        metrics_.active_operation_count += 1;

        return OperationHandle<type>(operation);
    }

}
