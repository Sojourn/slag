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

void slag::Reactor::startup() {
}

void slag::Reactor::shutdown() {
    for (ResourceContext* resource_context: resource_contexts_) {
        if (resource_context->has_resource()) {
            throw std::runtime_error("Reactor cannot shutdown because a resource is still referencing it");
        }
    }

    while (!resource_contexts_.empty()) {
        step();
    }
}

slag::ResourceContext& slag::Reactor::allocate_resource_context(Resource& resource) {
    return *(new ResourceContext{resource});
}

void slag::Reactor::cleanup_resource_context(ResourceContext& resource_context) {
    for (Operation* operation: resource_context.operations()) {
        cancel_operation(*operation);
    }

    // TODO: close file descriptor
}

void slag::Reactor::deallocate_resource_context(ResourceContext& resource_context) {
    delete &resource_context;
}

void slag::Reactor::attach_resource(Resource& resource) {
    assert(!resource.has_resource_context());

    ResourceContext& resource_context = allocate_resource_context(resource);
    resource.set_resource_context(&resource_context);
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
