#pragma once

#include <bitset>
#include <vector>
#include "slag/operation.h"
#include "slag/file_descriptor.h"

namespace slag {

    class Resource;
    class Operation;

    class ResourceContext {
    public:
        explicit ResourceContext(Resource& resource);
        ResourceContext(ResourceContext&&) noexcept = delete;
        ResourceContext(const ResourceContext&) = delete;
        ~ResourceContext();

        ResourceContext& operator=(ResourceContext&&) noexcept = delete;
        ResourceContext& operator=(const ResourceContext&) = delete;

    private:
        friend class Reactor;

        [[nodiscard]] bool has_resource() const;
        [[nodiscard]] Resource& resource();
        [[nodiscard]] const Resource& resource() const;
        void remove_resource();
        void update_resource(Resource& resource);

        [[nodiscard]] bool has_deferred_action(OperationAction operation_action) const;
        void set_deferred_action(OperationAction operation_action);
        void reset_deferred_action(OperationAction operation_action);
        bool update_deferred_action(OperationAction operation_action);

        [[nodiscard]] std::vector<Operation*>& operations();
        [[nodiscard]] const std::vector<Operation*>& operations() const;

        [[nodiscard]] FileDescriptor& file_descriptor();
        [[nodiscard]] const FileDescriptor& file_descriptor() const;

        [[nodiscard]] bool is_referenced() const;

    private:
        Resource*                           resource_;
        std::bitset<OPERATION_ACTION_COUNT> deferred_actions_;
        std::vector<Operation*>             operations_;
        FileDescriptor                      file_descriptor_;
    };

}
