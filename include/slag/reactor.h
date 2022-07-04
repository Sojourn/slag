#pragma once

#include <chrono>
#include <vector>
#include <unordered_set>
#include "slag/operation.h"
#include "slag/resource_context.h"
#include "slag/resource_context_index.h"

namespace slag {

    class Resource;
    class ResourceContext;

    class Reactor {
    public:
        Reactor();
        Reactor(Reactor&&) noexcept = delete;
        Reactor(const Reactor&) = delete;
        ~Reactor();

        Reactor& operator=(Reactor&&) noexcept = delete;
        Reactor& operator=(const Reactor&) = delete;

    protected:
        void complete_operation(Operation& operation, int64_t result);
        void handle_operation_event(Operation& operation, OperationEvent operation_event);
        void defer_operation_action(Operation& operation, OperationAction operation_action);

        ResourceContextIndex<OperationAction::SUBMIT>::Cursor deferred_submit_operation_actions();
        ResourceContextIndex<OperationAction::NOTIFY>::Cursor deferred_notify_operation_actions();

    private:
        friend class EventLoop;

        virtual void startup();
        virtual void step() = 0;
        virtual void shutdown();

        virtual ResourceContext& allocate_resource_context(Resource& resource);
        virtual void cleanup_resource_context(ResourceContext& resource_context);
        virtual void deallocate_resource_context(ResourceContext& resource_context);

    private:
        friend class Resource;

        void attach_resource(Resource& resource);
        void move_resource(Resource& target_resource, Resource& source_resource);
        void detach_resource(Resource& resource);

        template<OperationType operation_type>
        Operation& start_operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> operation_parameters);
        void cancel_operation(Operation& operation);

    private:
        ResourceContextIndex<OperationAction::SUBMIT> submit_resource_context_index_;
        ResourceContextIndex<OperationAction::NOTIFY> notify_resource_context_index_;
        ResourceContextIndex<OperationAction::REMOVE> remove_resource_context_index_;

        // TODO: use a intrusive_list
        std::unordered_set<ResourceContext*> resource_contexts_;
    };

    [[nodiscard]] Reactor& local_reactor();

    template<OperationType operation_type>
    inline Operation& Reactor::start_operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> operation_parameters) {
        Operation* operation = new Operation{resource_context, user_data, std::move(operation_parameters)};
        resource_context.operations().push_back(operation);
        defer_operation_action(*operation, operation->action()); // defer submission
        return *operation;
    }

}
