#include "slag/reactor.h"
#include "slag/resource.h"
#include "slag/resource_context.h"
#include "slag/operation.h"
#include "slag/operation_parameters.h"
#include "slag/event_loop.h"
#include "slag/logging.h"
#include <algorithm>
#include <stdexcept>
#include <cassert>

slag::Reactor::Reactor()
    : submit_resource_context_index_{OperationAction::SUBMIT}
    , notify_resource_context_index_{OperationAction::NOTIFY}
    , remove_resource_context_index_{OperationAction::REMOVE}
    , operation_allocator_{4096} // TODO: make configurable
    , resource_context_count_{0}
    , attached_resource_count_{0}
{
}

slag::Reactor::~Reactor() {
    assert(!resource_context_count_);
}

void slag::Reactor::complete_operation(Operation& operation, int64_t result) {
    info(
        "Reactor/{} complete_operation Operation/{} result/{}"
        , (const void*)this
        , (const void*)&operation
        , result
    );

    operation.set_result(result);
    handle_operation_event(operation, OperationEvent::COMPLETION);
}

void slag::Reactor::handle_operation_event(Operation& operation, OperationEvent operation_event) {
    OperationState original_state = operation.state();
    operation.state_machine().handle_event(operation_event);

    info(
        "Operation/{} {} + {} -> {}"
        , (const void*)&operation
        , to_string(original_state)
        , to_string(operation_event)
        , to_string(operation.state())
    );

    defer_operation_action(operation, operation.action());
}

void slag::Reactor::defer_operation_action(Operation& operation, OperationAction operation_action) {
    assert(operation_action == operation.action());
    if (operation_action == OperationAction::PANIC) {
        abort();
    }

    info(
        "Reactor/{} defer_operation_action Operation/{} action/{}"
        , (const void*)this
        , (const void*)&operation
        , to_string(operation_action) 
    );

    ResourceContext& resource_context = operation.resource_context();
    if (resource_context.has_deferred_action(operation_action)) {
        return;
    }

    switch (operation_action) {
        case OperationAction::WAIT: {
            // not handled
            break;
        }
        case OperationAction::SUBMIT: {
            submit_resource_context_index_.insert(resource_context);
            break;
        }
        case OperationAction::NOTIFY: {
            notify_resource_context_index_.insert(resource_context);
            break;
        }
        case OperationAction::REMOVE: {
            remove_resource_context_index_.insert(resource_context);
            break;
        }
        case OperationAction::PANIC: {
            // handled above
            break;
        }
    }
}

slag::ResourceContextIndex::Cursor slag::Reactor::deferred_submit_operation_actions() {
    return submit_resource_context_index_.select();
}

slag::ResourceContextIndex::Cursor slag::Reactor::deferred_notify_operation_actions() {
    return notify_resource_context_index_.select();
}

void slag::Reactor::notify() {
    auto cursor = deferred_notify_operation_actions();
    while (ResourceContext* resource_context = cursor.next()) {
        size_t operation_count = resource_context->operations().size();
        for (size_t operation_index = 0; operation_index < operation_count; ++operation_index) {
            Operation* operation = resource_context->operations()[operation_index];
            if (operation->action() != OperationAction::NOTIFY) {
                continue;
            }

            handle_operation_event(*operation, OperationEvent::NOTIFICATION);
            if (operation->test_flag(OperationFlag::INTERNAL)) {
                handle_internal_operation_complete(*operation);
            }
            else if (resource_context->has_resource()) {
                resource_context->resource().handle_operation_complete(*operation);
            }
        }
    }
}

void slag::Reactor::startup() {
    info(
        "Reactor/{} startup beginning"
        , (const void*)this
    );

    info(
        "Reactor/{} startup complete"
        , (const void*)this
    );
}

void slag::Reactor::step() {
    garbage_collect();
}

void slag::Reactor::shutdown() {
    info(
        "Reactor/{} shutdown beginning"
        , (const void*)this
    );

    if (attached_resource_count_) {
        throw std::runtime_error("Cannot shutdown while a resource is still attached");
    }

    while (resource_context_count_) {
        step();
    }

    info(
        "Reactor/{} shutdown complete"
        , (const void*)this
    );
}

void slag::Reactor::cleanup_resource_context(ResourceContext& resource_context) {
    for (Operation* operation: resource_context.operations()) {
        cancel_operation(*operation);
    }

    // TODO: close file descriptor
}

void slag::Reactor::garbage_collect() {
    submit_resource_context_index_.vacuum();
    notify_resource_context_index_.vacuum();

    {
        auto cursor = remove_resource_context_index_.select();
        while (ResourceContext* resource_context = cursor.next()) {
            resource_context->reset_deferred_action(OperationAction::REMOVE);

            garbage_collect(*resource_context);
        }
    }

    remove_resource_context_index_.truncate();
}

void slag::Reactor::garbage_collect(ResourceContext& resource_context) {
    garbage_collect(resource_context.operations());

    if (!resource_context.is_referenced()) {
        --resource_context_count_;
        deallocate_resource_context(resource_context);
    }
}

void slag::Reactor::garbage_collect(std::vector<Operation*>& operations) {
    for (Operation*& operation: operations) {
        if (operation->action() == OperationAction::REMOVE) {
            delete operation;
            operation = nullptr;
        }
    }

    auto beg = operations.begin();
    auto end = operations.end();
    auto pos = std::remove_if(
        beg,
        end,
        [](Operation* operation) {
            return !operation;
        }
    );

    if (pos != end) {
        operations.erase(pos, end);
    }
}

void slag::Reactor::attach_resource(Resource& resource) {
    assert(!resource.has_resource_context());

    ResourceContext& resource_context = allocate_resource_context(resource);
    ++resource_context_count_;
    
    resource.set_resource_context(&resource_context);
    ++attached_resource_count_;
}

void slag::Reactor::move_resource(Resource& target_resource, Resource& source_resource) {
    assert(source_resource.has_resource_context());
    assert(&target_resource != &source_resource);

    if (target_resource.has_resource_context()) {
        detach_resource(target_resource);
    }

    ResourceContext& resource_context = source_resource.resource_context();
    source_resource.set_resource_context(nullptr);
    target_resource.set_resource_context(&resource_context);
    resource_context.update_resource(target_resource);
}

void slag::Reactor::detach_resource(Resource& resource) {
    assert(resource.has_resource_context());

    ResourceContext& resource_context = resource.resource_context();
    resource_context.remove_resource();
    resource.set_resource_context(nullptr);
    --attached_resource_count_;

    cleanup_resource_context(resource_context);
    remove_resource_context_index_.insert(resource_context);
}

void slag::Reactor::cancel_operation(Operation& operation) {
    if (operation.state() == OperationState::TERMINAL) {
        return; // this operation is doomed anyways
    }

    handle_operation_event(operation, OperationEvent::CANCEL);
}

slag::Reactor& slag::local_reactor() {
    return local_event_loop().reactor();
}
