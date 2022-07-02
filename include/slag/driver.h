#pragma once

#include <chrono>
#include <vector>
#include <unordered_set>
#include "slag/operation.h"

namespace slag {

    class Resource;
    class ResourceContext;

    // TODO: rename to Reactor
    class Driver {
    public:
        Driver(EventLoop& event_loop);
        Driver(Driver&&) noexcept = delete;
        Driver(const Driver&) = delete;
        ~Driver();

        Driver& operator=(Driver&&) noexcept = delete;
        Driver& operator=(const Driver&) = delete;

        [[nodiscard]] EventLoop& event_loop();
        [[nodiscard]] const EventLoop& event_loop() const;

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
        void defer_operation_action(Operation& operation);

    private:
        EventLoop&                    event_loop_;
        std::vector<ResourceContext*> deferred_submit_actions_;
        std::vector<ResourceContext*> deferred_notify_actions_;
        std::vector<ResourceContext*> deferred_remove_actions_;

        // TODO: use a intrusive_list
        std::unordered_set<ResourceContext*> resource_contexts_;
    };

}
