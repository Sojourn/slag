#include "slag/resource_context.h"
#include <cassert>

slag::ResourceContext::ResourceContext(Resource& resource)
    : resource_{&resource}
{
}

slag::ResourceContext::~ResourceContext() {
    assert(!is_referenced());
    assert(!file_descriptor_.is_open());
}

bool slag::ResourceContext::has_resource() const {
    return static_cast<bool>(resource_);
}

slag::Resource& slag::ResourceContext::resource() {
    assert(has_resource());
    return *resource_;
}

const slag::Resource& slag::ResourceContext::resource() const {
    assert(has_resource());
    return *resource_;
}

void slag::ResourceContext::remove_resource() {
    assert(has_resource());
    resource_ = nullptr;
}

void slag::ResourceContext::update_resource(Resource& resource) {
    assert(has_resource());
    resource_ = &resource;
}

bool slag::ResourceContext::has_deferred_action(OperationAction operation_action) const {
    return deferred_actions_.test(to_index(operation_action));
}

void slag::ResourceContext::set_deferred_action(OperationAction operation_action) {
    deferred_actions_.set(to_index(operation_action));
}

void slag::ResourceContext::reset_deferred_action(OperationAction operation_action) {
    deferred_actions_.reset(to_index(operation_action));
}

bool slag::ResourceContext::update_deferred_action(OperationAction operation_action) {
    reset_deferred_action(operation_action);
    for (const Operation& operation: operations_) {
        if (operation.action() == operation_action) {
            set_deferred_action(operation_action);
            break;
        }
    }

    return has_deferred_action(operation_action);
}

std::vector<slag::Operation*>& slag::ResourceContext::operations() {
    return operations_;
}

const std::vector<slag::Operation*>& slag::ResourceContext::operations() const {
    return operations_;
}

slag::FileDescriptor& slag::ResourceContext::file_descriptor() {
    return file_descriptor_;
}

const slag::FileDescriptor& slag::ResourceContext::file_descriptor() const {
    return file_descriptor_;
}

bool slag::ResourceContext::is_referenced() const {
    return resource_ || deferred_actions_.any() || !operations_.empty();
}
