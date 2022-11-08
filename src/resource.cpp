#include "slag/resource.h"
#include "slag/resource_context.h"
#include "slag/operation.h"
#include "slag/reactor.h"
#include "slag/event_loop.h"
#include <cassert>

slag::Resource::Resource()
    : resource_context_{nullptr}
{
}

slag::Resource::Resource(Resource&& other) noexcept
    : Resource{}
{
    local_reactor().move_resource(*this, other);
}

slag::Resource::~Resource() {
    if (has_resource_context()) {
        local_reactor().detach_resource(*this);
        assert(!resource_context_);
    }
}

slag::Resource& slag::Resource::operator=(Resource&& rhs) noexcept {
    if (this != &rhs) {
        if (resource_context_) {
            local_reactor().detach_resource(*this);
        }
        if (rhs.resource_context_) {
            local_reactor().move_resource(*this, rhs);
        }
    }

    return *this;
}

slag::Operation& slag::Resource::start_nop_operation(void* user_data) {
    static constexpr OperationType operation_type = OperationType::NOP;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
        }
    );
}

slag::Operation& slag::Resource::start_assign_operation(void* user_data, FileDescriptor file_descriptor) {
    static constexpr OperationType operation_type = OperationType::ASSIGN;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
            .arguments = {
                .file_descriptor = std::move(file_descriptor),
            },
            .result = {},
        }
    );
}

slag::Operation& slag::Resource::start_bind_operation(void* user_data, const Address& address) {
    static constexpr OperationType operation_type = OperationType::BIND;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
            .arguments = {
                .address = address,
            },
            .result = {},
        }
    );
}

slag::Operation& slag::Resource::start_listen_operation(void* user_data, int backlog) {
    static constexpr OperationType operation_type = OperationType::LISTEN;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
            .arguments = {
                .backlog = backlog,
            },
            .result = {},
        }
    );
}

slag::Operation& slag::Resource::start_accept_operation(void* user_data) {
    static constexpr OperationType operation_type = OperationType::ACCEPT;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
            // pass
        }
    );
}

slag::Operation& slag::Resource::start_send_operation(void* user_data, BufferSlice buffer_slice) {
    static constexpr OperationType operation_type = OperationType::SEND;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
            .arguments = {
                std::move(buffer_slice),
            },
            .result = {}
        }
    );
}

slag::Operation& slag::Resource::start_receive_operation(void* user_data, size_t count) {
    static constexpr OperationType operation_type = OperationType::RECEIVE;

    return local_reactor().start_operation<operation_type>(
        resource_context(),
        user_data,
        OperationParameters<operation_type> {
            .arguments = {
                .count = count,
            },
            .buffer = make_handle<Buffer>(std::max(count, size_t{1})),
            .result = {}
        }
    );
}

void slag::Resource::cancel_operation(Operation& operation) {
    local_reactor().cancel_operation(operation);
}

bool slag::Resource::has_resource_context() const {
    return static_cast<bool>(resource_context_);
}

slag::ResourceContext& slag::Resource::resource_context() {
    if (!resource_context_) {
        local_reactor().attach_resource(*this);
        assert(resource_context_);
    }

    return *resource_context_;
}

void slag::Resource::set_resource_context(ResourceContext* resource_context) {
    resource_context_ = resource_context;
}
