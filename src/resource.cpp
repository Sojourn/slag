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
    return local_reactor().start_operation<OperationType::NOP>(
        resource_context(),
        user_data,
        OperationParameters<OperationType::NOP> {
        }
    );
}

slag::Operation& slag::Resource::start_assign_operation(void* user_data, FileDescriptor file_descriptor) {
    return local_reactor().start_operation<OperationType::ASSIGN>(
        resource_context(),
        user_data,
        OperationParameters<OperationType::ASSIGN> {
            .arguments = {
                .file_descriptor = std::move(file_descriptor),
            },
            .result = {},
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
