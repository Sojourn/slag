#pragma once

#include <array>
#include <liburing.h>
#include "slag/core/selector.h"
#include "slag/core/executor.h"
#include "slag/system/operation.h"
#include "slag/system/operation_handle.h"
#include "slag/system/operation_allocator.h"

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

        template<OperationType operation_type, typename... Args>
        OperationHandle<operation_type> start_operation(Args&&... args);

        void poll(bool non_blocking);

        // Returns true if there are no active operations.
        bool is_quiescent() const;

        void set_interrupt_handler(InterruptHandler& interrupt_handler);

        // TODO: rename to borrow_file_descriptor.
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
        void handle_daemonized(OperationBase& operation_base);

    private:
        struct OperationMetrics {
            std::array<size_t, OPERATION_TYPE_COUNT> active_counts;
            std::array<size_t, OPERATION_TYPE_COUNT> daemon_counts;

            size_t total_active_count() const;
            size_t total_daemon_count() const;
        };

        struct Metrics {
            OperationMetrics operations;
        };

        void increment_operation_count(const OperationBase& operation_base);
        void decrement_operation_count(const OperationBase& operation_base);

    private:
        struct io_uring          ring_;
        Executor&                executor_;
        InterruptHandler*        interrupt_handler_;
        Selector                 pending_submissions_;
        Selector                 garbage_;
        Metrics                  metrics_;
        OperationAllocator       operation_allocator_;
    };

}

#include "reactor.hpp"
