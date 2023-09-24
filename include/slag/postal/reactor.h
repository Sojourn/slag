#pragma once

#include <liburing.h>        
#include "slag/core/selector.h"
#include "slag/core/executor.h"
#include "slag/postal/operation.h"
#include "slag/postal/operation_handle.h"
#include "slag/postal/operation_allocator.h"

namespace slag {

    class InterruptHandler {
    protected:
        ~InterruptHandler() = default;

    public:
        virtual void handle_interrupt(uint16_t source, uint16_t reason) = 0;
    };

    class Reactor {
        Reactor(Reactor&&) = delete;
        Reactor(const Reactor&) = delete;
        Reactor& operator=(Reactor&&) = delete;
        Reactor& operator=(const Reactor&) = delete;

    public:
        explicit Reactor(Executor& executor);
        ~Reactor();

        template<OperationType type, typename... Args>
        OperationHandle<type> start_operation(Args&&... args);

        void poll(bool non_blocking);

        // Returns true if there are no active operations.
        bool is_quiescent() const;

        void set_interrupt_handler(InterruptHandler& interrupt_handler);

        int file_descriptor();

    private:
        size_t prepare_pending_operations();
        void process_completions();
        void process_completion(struct io_uring_cqe& cqe);

        void process_operation_completion(struct io_uring_cqe& cqe);
        void process_interrupt_completion(struct io_uring_cqe& cqe);

        void collect_garbage();

    private:
        friend class OperationBase;

        void handle_abandoned(OperationBase& operation_base);

    private:
        struct Metrics {
            size_t total_operation_count = 0;
            size_t active_operation_count = 0;
        };

        struct io_uring          ring_;
        Executor&                executor_;
        Selector                 pending_submissions_;

        Metrics                  metrics_;
        Selector                 garbage_;
        OperationAllocator       operation_allocator_;

        InterruptHandler*        interrupt_handler_;
    };

}

#include "reactor.hpp"
