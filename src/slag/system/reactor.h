#pragma once

#include <array>
#include <liburing.h>
#include "slag/core.h"
#include "operation.h"
#include "interrupt.h"

namespace slag {

    class Reactor {
        Reactor(Reactor&&) = delete;
        Reactor(const Reactor&) = delete;
        Reactor& operator=(Reactor&&) = delete;
        Reactor& operator=(const Reactor&) = delete;

    public:
        explicit Reactor(InterruptHandler& interrupt_handler);
        ~Reactor();

        // Returns a file descriptor that can be used to notify this ring.
        int borrow_file_descriptor();

        // Schedule an operation for submission to the kernel.
        void schedule(Operation& operation);

        // Returns true if any operations completed.
        // Optionally block until one completes, or we receive an interrupt.
        bool poll(bool non_blocking);

    private:
        size_t prepare_submissions();
        void prepare_submission(struct io_uring_sqe& sqe);

        size_t process_completions();
        void process_completion(struct io_uring_cqe& cqe);

        void process_operation_completion(struct io_uring_cqe& cqe, const OperationUserData& user_data);
        void process_interrupt_completion(struct io_uring_cqe& cqe, const OperationUserData& user_data);

    private:
        InterruptHandler& interrupt_handler_;
        ResourceTable&    operation_table_;
        ThreadIndex       thread_index_;

        struct io_uring   ring_;
        Selector          pending_submissions_;
    };

}
