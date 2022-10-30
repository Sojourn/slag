#pragma once

#include <span>
#include <optional>
#include <cstdint>
#include <cstddef>
#include "slag/file_descriptor.h"
#include "slag/byte_stream.h"
#include "slag/handle.h"
#include "slag/address.h"

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
        Operation& start_assign_operation(void* user_data, FileDescriptor file_descriptor);
        Operation& start_bind_operation(void* user_data, const Address& address);
        Operation& start_listen_operation(void* user_data, int backlog);
        Operation& start_accept_operation(void* user_data);
        Operation& start_send_operation(void* user_data, std::span<const std::byte> data, Handle<Buffer> buffer);
        Operation& start_receive_operation(void* user_data, std::optional<size_t> count = {});
        void cancel_operation(Operation& operation);

        virtual void handle_operation_complete(Operation& operation) = 0;

    private:
        friend class Reactor;
        friend class IOURingReactor;

        [[nodiscard]] bool has_resource_context() const;
        [[nodiscard]] ResourceContext& resource_context();
        void set_resource_context(ResourceContext* resource_context);

    private:
        ResourceContext* resource_context_;
    };

}
