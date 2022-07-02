#include "slag/resource_context_index.h"
#include <algorithm>

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor(ResourceContextIndex& index) {
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor() {
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor(Cursor&& other) noexcept {
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor(const Cursor& other) {
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::~Cursor() {
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor& slag::ResourceContextIndex<operation_action>::Cursor::operator=(Cursor&& rhs) noexcept {
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor& slag::ResourceContextIndex<operation_action>::Cursor::operator=(const Cursor& rhs) {
}

template<OperationAction operation_action>
slag::ResourceContext* slag::ResourceContextIndex<operation_action>::Cursor::next_resource_context() {
    if (!parent_) {
        return nullptr;
    }

    if (parent_->resource_contexts_.size() <= resource_context_index_) {
        reset();
        return nullptr;
    }

    return parent_->resource_contexts_[resource_context_index_++];
}

template<OperationAction operation_action>
slag::Operation* slag::ResourceContextIndex<operation_action>::Cursor::next_operation() {
    if (!parent_) {
        return nullptr;
    }

    // TODO: think about how nested iteration should work

    auto&& resource_context = parent_->resource_contexts_[resource_context_index_];
    auto&& operations = resource_context.operations();
    if (operations.size() <= operation_index_) {
        return nullptr;
    }
}

template<OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::Cursor::reset() {
    if (!parent_) {
        return; // not attached
    }

    --parent_->cursor_count_; // decref

    parent_ = nullptr;
    resource_context_index_ = 0;
    operation_index_ = 0;
}

template<OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor slag::ResourceContextIndex<operation_action>::select() {
    return Cursor{*this};
}

template<OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::insert(ResourceContext& resource_context) {
    if (resource_context.has_deferred_action(operation_action)) {
        return; // already in the index
    }

    resource_context.set_deferred_action(operation_action);
    resource_contexts_.push_back(&resource_context);
}

template<OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::vacuum() {
    if (cursor_count_) {
        return; // cannot vacuum while a select is in progress
    }

    for (ResourceContext*& resource_context: resource_contexts_) {
        if (!resource_context.update_deferred_action(operation_action)) {
            resource_context = nullptr;
        }
    }

    auto beg = resource_contexts_.begin();
    auto end = resource_contexts_.end();
    auto pos = std::remove_if(beg, end, []() {
        return !resource_context;
    });

    resource_contexts_.erase(pos, end);
}

template<OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::truncate() {
    if (cursor_count_) {
        throw std::runtime_error("ResourceContextIndex cannot be truncated while a select is in progress");
    }

    resource_contexts_.clear();
}
