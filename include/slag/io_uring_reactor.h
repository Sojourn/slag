#pragma once

#include <liburing.h>        
#include "slag/reactor.h"
#include "slag/resource.h"
#include "slag/resource_context.h"
#include "slag/io_uring_resource_context.h"
#include "slag/pool_allocator.h"

namespace slag {

    class IOURingReactor : public Reactor {
    public:
        IOURingReactor();
        ~IOURingReactor();

    private:
        void startup() override;
        void step() override;
        void shutdown() override;

        ResourceContext& allocate_resource_context(Resource& resource) override;
        void cleanup_resource_context(ResourceContext& resource_context) override;
        void deallocate_resource_context(ResourceContext& resource_context) override;

    private:
        void process_submissions();
        void process_completions();

        template<OperationType operation_type>
        void submit_operation(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<operation_type>& operation_parameters);
        void submit_cancel(struct io_uring_sqe& sqe, Operation& operation);

    private:
        struct io_uring                       ring_;
        PoolAllocator<IOURingResourceContext> resource_context_allocator_;
    };

}
