#include "slag/reactor.h"
#include "slag/resource.h"
#include "slag/resource_context.h"
#include "slag/operation.h"
#include "slag/operation_parameters.h"
#include "slag/event_loop.h"
#include <stdexcept>

slag::Reactor::Reactor() {
}

slag::Reactor::~Reactor() {
    // TODO: assert that we have shutdown cleanly
}

void slag::Reactor::handle_operation_event(Operation& operation, OperationEvent operation_event) {
    operation.state_machine().handle_event(operation_event);
    defer_operation_action(operation, operation.action());
}

void slag::Reactor::defer_operation_action(Operation& operation, OperationAction operation_action) {
    assert(operation_action == operation.action());
    if (operation_action == OperationAction::PANIC) {
        abort();
    }

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
            resource_context.set_deferred_action(operation_action);
            deferred_notify_actions_.push_back(&resource_context);
            break;
        }
        case OperationAction::NOTIFY: {
            resource_context.set_deferred_action(operation_action);
            deferred_notify_actions_.push_back(&resource_context);
            break;
        }
        case OperationAction::REMOVE: {
            resource_context.set_deferred_action(operation_action);
            deferred_remove_actions_.push_back(&resource_context);
            break;
        }
        case OperationAction::PANIC: {
            // handled above
            break;
        }
    }
}

void slag::Reactor::startup() {
}

void slag::Reactor::shutdown() {
    for (ResourceContext* resource_context: resource_contexts_) {
        if (resource_context->has_resource()) {
            throw std::runtime_error("Reactor cannot shutdown because a resource is still referencing it");
        }

        for (Operation* operation: resource_context->operations()) {
            cancel_operation(*operation);
        }
    }

    while (!resource_contexts_.empty()) {
        step();
    }
}

void slag::Reactor::attach_resource(Resource& resource) {
    assert(!resource.has_resource_context());
}

void slag::Reactor::move_resource(Resource& target_resource, Resource& source_resource) {
    assert(source_resource.has_resource_context());
    assert(&target_resource != &source_resource);
}

void slag::Reactor::detach_resource(Resource& resource) {
    assert(resource.has_resource_context());
}

template<slag::OperationType operation_type>
slag::Operation& slag::Reactor::start_operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> operation_parameters) {
    Operation* operation = new Operation{resource_context, user_data, std::move(operation_parameters)};
    resource_context.operations().push_back(operation);
    defer_operation_action(*operation, operation->action()); // defer submission
    return *operation;
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

// explicit templated function instantiation
#define X(SLAG_OPERATION_TYPE)                                                                                                                                                                                          \
template<>                                                                                                                                                                                                              \
slag::Operation& slag::Reactor::start_operation<slag::OperationType::SLAG_OPERATION_TYPE>(ResourceContext& resource_context, void* user_data, OperationParameters<slag::OperationType::SLAG_OPERATION_TYPE> parameters);

SLAG_OPERATION_TYPES(X)
#undef X
