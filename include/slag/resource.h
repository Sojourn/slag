#pragma once

namespace slag {

    class Operation;
    class EventLoop;

    class Resource {
    public:
        explicit Resource(EventLoop& event_loop);
        Resource(Resource&& other) noexcept;
        Resource(const Resource&) = delete;
        virtual ~Resource();

        Resource& operator=(Resource&& rhs) noexcept;
        Resource& operator=(const Resource&) = delete;

        [[nodiscard]] EventLoop& event_loop();
        [[nodiscard]] const EventLoop& event_loop() const;

        // Operation& start_nop_operation(void* user_data);

        virtual void handle_operation_complete(Operation& operation) = 0;

    private:
        friend class Driver;

        [[nodiscard]] ResourceContext& resource_context();
        void set_resource_context(ResourceContext* resource_context);

    private:
        EventLoop*       event_loop_;
        ResourceContext* resource_context_;
    };

}
