#pragma once

#include <chrono>
#include <vector>
#include <unordered_set>
#include "slag/operation.h"

namespace slag {

    class Resource;
    class ResourceContext;

    class Reactor {
    public:
        Reactor(EventLoop& event_loop);
        Reactor(Reactor&&) noexcept = delete;
        Reactor(const Reactor&) = delete;
        ~Reactor();

        Reactor& operator=(Reactor&&) noexcept = delete;
        Reactor& operator=(const Reactor&) = delete;

        [[nodiscard]] EventLoop& event_loop();
        [[nodiscard]] const EventLoop& event_loop() const;

    protected:
        void complete_operation(Operation& operation, int64_t result);
        void handle_operation_event(Operation& operation, OperationEvent operation_event);
        void defer_operation_action(Operation& operation, OperationAction operation_action);

    private:
        friend class EventLoop;

        virtual void startup();
        virtual void step(std::chrono::milliseconds timeout) = 0;
        virtual void shutdown();

    private:
        friend class Resource;

        void attach_resource(Resource& resource);
        void move_resource(Resource& target_resource, Resource& source_resource)
        void detach_resource(Resource& resource);

        template<OperationType>
        Operation& start_operation(ResourceContext& resource_context, void* user_data, OperationParams<type> operation_params);
        void cancel_operation(Operation& operation);

    private:
        EventLoop&                    event_loop_;
        std::vector<ResourceContext*> deferred_submit_actions_;
        std::vector<ResourceContext*> deferred_notify_actions_;
        std::vector<ResourceContext*> deferred_remove_actions_;

        // TODO: use a intrusive_list
        std::unordered_set<ResourceContext*> resource_contexts_;
    };

}
