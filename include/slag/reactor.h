#pragma once

#include <vector>
#include "slag/operation.h"
#include "slag/resource_context.h"
#include "slag/resource_context_index.h"
#include "slag/pool_allocator.h"

namespace slag {

    class Resource;
    class ResourceContext;

    class Reactor {
    public:
        Reactor();
        Reactor(Reactor&&) noexcept = delete;
        Reactor(const Reactor&) = delete;
        virtual ~Reactor();

        Reactor& operator=(Reactor&&) noexcept = delete;
        Reactor& operator=(const Reactor&) = delete;

    protected:
        void complete_operation(Operation& operation, int64_t result);
        void handle_operation_event(Operation& operation, OperationEvent operation_event);
        void defer_operation_action(Operation& operation, OperationAction operation_action);

        ResourceContextIndex::Cursor deferred_submit_operation_actions();
        ResourceContextIndex::Cursor deferred_notify_operation_actions();

        void notify();

        virtual void startup();
        virtual void step();
        virtual void shutdown();

        virtual [[nodiscard]] ResourceContext& allocate_resource_context(Resource& resource) = 0;
        virtual void cleanup_resource_context(ResourceContext& resource_context);
        virtual void deallocate_resource_context(ResourceContext& resource_context) = 0;

        virtual void handle_internal_operation_complete(Operation& operation) = 0;

    private:
        friend class EventLoop;

        void garbage_collect();
        void garbage_collect(ResourceContext& resource_context);
        void garbage_collect(std::vector<Operation*>& operations);

    private:
        friend class Resource;

        void attach_resource(Resource& resource);
        void move_resource(Resource& target_resource, Resource& source_resource);
        void detach_resource(Resource& resource);

        template<OperationType operation_type>
        Operation& start_operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> operation_parameters);
        void cancel_operation(Operation& operation);

    private:
        ResourceContextIndex     submit_resource_context_index_;
        ResourceContextIndex     notify_resource_context_index_;
        ResourceContextIndex     remove_resource_context_index_;
        PoolAllocator<Operation> operation_allocator_;
        size_t                   resource_context_count_;
        size_t                   attached_resource_count_;
    };

    [[nodiscard]] Reactor& local_reactor();

    template<OperationType operation_type>
    inline Operation& Reactor::start_operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> operation_parameters) {
        Operation& operation = operation_allocator_.allocate(resource_context, user_data, std::move(operation_parameters));
        resource_context.operations().push_back(&operation);
        defer_operation_action(operation, operation.action()); // defer submission
        return operation;
    }

}
