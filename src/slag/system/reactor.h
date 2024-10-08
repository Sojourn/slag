#pragma once

#include <array>
#include <liburing.h>
#include "slag/core.h"
#include "operation.h"
#include "operation_table.h"
#include "interrupt.h"

namespace slag {

    class Reactor {
        Reactor(Reactor&&) = delete;
        Reactor(const Reactor&) = delete;
        Reactor& operator=(Reactor&&) = delete;
        Reactor& operator=(const Reactor&) = delete;

    public:
        Reactor();
        ~Reactor();

        // Returns a file descriptor that can be used to notify this ring.
        int borrow_file_descriptor();

        // The reactor can be interrupted (woken) for various reasons. The interrupt vector
        // contains events that can be used by handlers.
        InterruptVector& interrupt_vector();
        InterruptState& interrupt_state(InterruptReason reason);

        template<typename OperationImpl, typename... Args>
        Ref<OperationImpl> create_operation(Args&&... args);
        void schedule_operation(Operation& operation);
        void destroy_operation(Operation& operation);

        // Returns true if any operations completed.
        // Optionally block until one completes, or we receive an interrupt.
        bool poll(bool non_blocking);

    private:
        size_t prepare_submissions();
        void prepare_submission(struct io_uring_sqe& io_sqe);

        size_t process_completions();
        void process_completion(struct io_uring_cqe& io_cqe);

        void process_operation_completion(struct io_uring_cqe& io_cqe, OperationKey op_key);
        void process_interrupt_completion(struct io_uring_cqe& io_cqe, OperationKey op_key);

    private:
        struct io_uring ring_;
        Selector        pending_submissions_;
        OperationTable  submitted_operation_table_;
        InterruptVector interrupt_vector_;
    };

    template<typename OperationImpl, typename... Args>
    Ref<OperationImpl> Reactor::create_operation(Args&&... args) {
        return bind(
            *(new OperationImpl(std::forward<Args>(args)...))
        );
    }

}
