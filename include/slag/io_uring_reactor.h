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

        void handle_internal_operation_complete(Operation& operation) override;

    private:
        template<OperationType operation_type>
        struct Subject {
            IOURingResourceContext&              resource_context;
            Operation&                           operation;
            OperationParameters<operation_type>& operation_parameters;
        };

        void process_submissions();

        template<OperationType operation_type>
        bool prepare_submission(Subject<operation_type>& subject);

        template<OperationType operation_type>
        bool prepare_cancel_submission(Subject<operation_type>& subject);

    private:
        void process_completions();
        void process_completion(Subject<OperationType::ASSIGN>& subject, int64_t result);
        void process_completion(Subject<OperationType::BIND>& subject, int64_t result);
        void process_completion(Subject<OperationType::LISTEN>& subject, int64_t result);
        void process_completion(Subject<OperationType::ACCEPT>& subject, int64_t result);

        template<OperationType operation_type>
        void process_completion(Subject<operation_type>& subject, int64_t result);

    private:
        struct io_uring                       ring_;
        PoolAllocator<IOURingResourceContext> resource_context_allocator_;
    };

}
