#pragma once

#include <liburing.h>        
#include "slag/postal/selector.h"
#include "slag/postal/executor.h"
#include "slag/postal/operation.h"
#include "slag/postal/operation_handle.h"
#include "slag/postal/operation_allocator.h"

namespace slag::postal {

    class Reactor {
        Reactor(Reactor&&) = delete;
        Reactor(const Reactor&) = delete;
        Reactor& operator=(Reactor&&) = delete;
        Reactor& operator=(const Reactor&) = delete;

    public:
        Reactor();
        ~Reactor();

        template<OperationType type, typename... Args>
        OperationHandle<type> start_operation(Args&&... args);

        void poll();

    private:
        size_t prepare_pending_operations();
        void process_completions();
        void process_completion(struct io_uring_cqe& cqe);
        void collect_garbage();

    private:
        friend class OperationBase;

        void handle_abandoned(OperationBase& operation_base);

    private:
        struct io_uring    ring_;
        Executor           executor_;
        Selector           pending_submissions_;
        Selector           garbage_;
        OperationAllocator operation_allocator_;
    };

}
