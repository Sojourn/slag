#include "slag/driver.h"
#include "slag/resource.h"
#include "slag/resource_context.h"
#include "slag/operation.h"
#include "slag/operation_parameters.h"
#include "slag/event_loop.h"
#include <stdexcept>

slag::Driver::Driver(EventLoop& event_loop)
    : event_loop_{event_loop}
{
}

slag::Driver::~Driver() {
    // TODO: assert that we have shutdown cleanly
}

slag::EventLoop& slag::Driver::event_loop() {
    return event_loop_;
}

const slag::EventLoop& slag::Driver::event_loop() const {
    return event_loop_;
}

void slag::Driver::startup() {
}

void slag::Driver::step() {
}

void slag::Driver::shutdown() {
    for (ResourceContext* resource_context: resource_contexts_) {
        if (resource_context->has_resource()) {
            throw std::runtime_error("Driver cannot shutdown because a resource is still referencing it");
        }

        for (Operation* operation: resource_context->operations) {
            cancel_operation(*operation);
        }
    }

    while (!resource_contexts_.empty()) {
        step();
    }
}

void slag::Driver::attach_resource(Resource& resource) {
    assert(!resource.has_resource_context());
}

void slag::Driver::move_resource(Resource& target_resource, Resource& source_resource) {
    assert(source_resource.has_resource_context());
    assert(&target_resource != &source_resource);
}

void slag::Driver::detach_resource(Resource& resource) {
    assert(resource.has_resource_context());
}

template<slag::OperationType>
slag::Operation& slag::Driver::start_operation(ResourceContext& resource_context, void* user_data, OperationParameters<type> operation_parameters) {
    Operation* operation = new Operation{resource_context, user_data, std::move(operation_parameters)};
    resource_context.operations.push_back(operation);
    defer_operation_action(*operation); // defer submission
    return *operation;
}

void slag::Driver::cancel_operation(Operation& operation) {
    if (operation.state() == OperationState::TERMINAL) {
        return; // this operation is doomed anyways
    }

    operation.state_machine().handle_event(OperationEvent::CANCEL);
    defer_operation_action(operation);
}

void slag::Driver::defer_operation_action(Operation& operation) {
    OperationAction operation_action = operation.action();
    if (operation_action == OperationAction::PANIC) {
        abort();
    }

    ResourceContext& resource_context = operation.resource_context();
    if (resource_context.has_deferred_action(operation_action)) {
        return;
    }

    switch (operation.action()) {
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

// explicit templated function instantiation
#define X(SLAG_OPERATION_TYPE)                                                                                                                                                                                          \
template<>                                                                                                                                                                                                              \
slag::Operation& slag::Driver::start_operation<slag::OperationType::SLAG_OPERATION_TYPE>(ResourceContext& resource_context, void* user_data, OperationParameters<slag::OperationType::SLAG_OPERATION_TYPE> parameters);

SLAG_OPERATION_TYPES(X)
#undef X
