#pragma once

#include <vector>

namespace slag {

    class Resource;
    class ResourceContext;
    class Operation;

    class Driver {
    public:
        Driver();
        Driver(Driver&&) noexcept = delete;
        Driver(const Driver&) = delete;
        ~Driver();

        Driver& operator=(Driver&&) noexcept = delete;
        Driver& operator=(const Driver&) = delete;

        [[nodiscard]] EventLoop& event_loop();
        [[nodiscard]] const EventLoop& event_loop() const;

    private:
        friend class EventLoop;

        void setup();
        void step();
        void cleanup();

    private:
        friend class Resource;

        void attach_resource(Resource& resource);
        void move_resource(Resource& target_resource, Resource& source_resource)
        void detach_resource(Resource& resource);

        template<OperationType>
        Operation& start_operation(ResourceContext& resource_context, void* user_data, OperationParams<type> operation_params);
        void cancel_operation(Operation& operation);

    private:
        // ...

    private:
        EventLoop&                    event_loop_;
        size_t                        resource_context_count_;
        std::vector<ResourceContext*> deferred_submit_actions_;
        std::vector<ResourceContext*> deferred_notify_actions_;
    };

}
