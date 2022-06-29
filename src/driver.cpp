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
slag::Operation& slag::Driver::start_operation(ResourceContext& resource_context, void* user_data, OperationParams<type> operation_params) {
}

void slag::Driver::cancel_operation(Operation& operation) {
}
