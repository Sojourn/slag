#pragma once

#include <liburing.h>        
#include "reactor.h"

namespace slag {

    class IOURingReactor : public Reactor {
    public:
        IOURingReactor();
        ~IOURingReactor();

    private:
        void startup() override;
        void step() override;
        void shutdown() override;

    private:
        void process_submissions();
        void process_completions();

        template<OperationType operation_type>
        void submit_operation(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<operation_type>& operation_parameters);
        void submit_cancel(struct io_uring_sqe& sqe, Operation& operation);

    private:
        struct io_uring ring_;
    };

}
