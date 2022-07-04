#pragma once

namespace slag {

    class ResourceContext;
    class Operation;

    class Resource {
    public:
        Resource();
        Resource(Resource&& other) noexcept;
        Resource(const Resource&) = delete;
        virtual ~Resource();

        Resource& operator=(Resource&& rhs) noexcept;
        Resource& operator=(const Resource&) = delete;

    protected:
        Operation& start_nop_operation(void* user_data);
        void cancel_operation(Operation& operation);

        virtual void handle_operation_complete(Operation& operation) = 0;

    private:
        friend class Reactor;

        [[nodiscard]] bool has_resource_context() const;
        [[nodiscard]] ResourceContext& resource_context();
        void set_resource_context(ResourceContext* resource_context);

    private:
        ResourceContext* resource_context_;
    };

}
